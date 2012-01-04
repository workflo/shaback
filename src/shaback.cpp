#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include "shaback.h"
#include "RuntimeConfig.h"
#include "DeflateOutputStream.h"
#include "StandardOutputStream.h"
#include "FileOutputStream.h"
#include "DeflateInputStream.h"
#include "StandardInputStream.h"

using namespace std;

Shaback::Shaback(RuntimeConfig& config) :
  config(config), repository(Repository(config))
{
}

Shaback::~Shaback()
{
}

void Shaback::createRepository()
{
  if (!config.force) {
    if (config.filesDir.isDir() || config.indexDir.isDir() || config.locksDir.isDir()) {
      cerr << "Looks like a shaback repository already: " << config.repository << endl;
      exit(3);
    }
    if (!config.repoDir.listFiles("*").empty()) {
      cerr << "Destination directory is not empty: " << config.repository << endl;
      exit(3);
    }
  }

  if (!config.filesDir.isDir()) {
    if (!config.filesDir.mkdir()) {
      cerr << "Cannot create directory: " << config.filesDir.path.c_str() << endl;
      exit(3);
    }
  }
  if (!config.indexDir.isDir()) {
    if (!config.indexDir.mkdir()) {
      cerr << "Cannot create directory: " << config.indexDir.path.c_str() << endl;
      exit(3);
    }
  }
  if (!config.locksDir.isDir()) {
    if (!config.locksDir.mkdir()) {
      cerr << "Cannot create directory: " << config.locksDir.path.c_str() << endl;
      exit(3);
    }
  }
  if (!config.cacheDir.isDir()) {
    if (!config.cacheDir.mkdir()) {
      cerr << "Cannot create directory: " << config.cacheDir.path.c_str() << endl;
      exit(3);
    }
  }

  char dirname[20];
  for (int level0 = 0; level0 <= 0xff; level0++) {
    sprintf(dirname, "%02x", level0);
    File dirLevel0(config.filesDir, dirname);
    dirLevel0.mkdir();

    for (int level1 = 0; level1 <= 0xff; level1++) {
      sprintf(dirname, "%02x", level1);
      File dirLevel1(dirLevel0, dirname);
      dirLevel1.mkdir();
    }
  }

  // Write default config:
  if (!config.repoPropertiesFile.isFile()) {
    string repoProperties = "# Don't modify this file after the first backup run! Otherwise loss of data is inevitable!\n\n"
        "version = 2\n\n"

        "# compression = None\n"
        "compression = Deflate\n\n"

        "encryption = None\n"
        "# encryption = Blowfish\n\n"

        "digest = SHA1\n";
    FileOutputStream os(config.repoPropertiesFile);
    os.write(repoProperties.data(), repoProperties.size());
  }

  cout << "Repository created: " << config.repository << endl;
}

int Shaback::deflate()
{
  StandardInputStream in(stdin);
  StandardOutputStream out(stdout);

  char buf[DEFLATE_CHUNK_SIZE];
  DeflateOutputStream def(&out);
  int bytesRead;

  while (true) {
    bytesRead = in.read(buf, DEFLATE_CHUNK_SIZE);
    if (bytesRead == -1)
      break;
    def.write(buf, bytesRead);
  }

  return 0;
}

int Shaback::inflate()
{
  StandardInputStream in(stdin);
  StandardOutputStream out(stdout);

  char buf[DEFLATE_CHUNK_SIZE];
  DeflateInputStream def(&in);
  int bytesRead;

  while (true) {
    bytesRead = def.read(buf, DEFLATE_CHUNK_SIZE);
    if (bytesRead == -1)
      break;
    out.write(buf, bytesRead);
  }

  return 0;
}
