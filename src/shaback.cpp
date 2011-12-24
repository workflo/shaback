#include <iostream>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>

#include "shaback.h"
#include "RuntimeConfig.h"

using namespace std;

Shaback::Shaback(RuntimeConfig& config)
  :config(config), repository(Repository(config))
{
}

Shaback::~Shaback()
{}


void Shaback::createRepository()
{
  if (!config.force) {
    if (config.filesDir.isDir() || config.indexDir.isDir() || config.locksDir.isDir()) {
      cerr << "Looks like a shaback repository already: " << config.repository << endl;
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
  
  // TODO: Repo-Konfig schreiben: Format-Version + Verschl�sselung + Kompressionsverfahren + Hashalgorithmus
  
  cout << "Repository created: " << config.repository << endl;
}
