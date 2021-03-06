/*
 * shaback - A hash digest based backup tool.
 * Copyright (C) 2012 Florian Wolff (florian@donuz.de)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include "lib/StandardInputStream.h"
#include "lib/StandardOutputStream.h"
#include "lib/FileOutputStream.h"
#include "lib/DeflateOutputStream.h"
#include "lib/DeflateInputStream.h"
#include "lib/BzInputStream.h"
#include "lib/BzOutputStream.h"
#include "lib/LzmaInputStream.h"
#include "lib/LzmaOutputStream.h"
#include "lib/ZStdInputStream.h"
#include "lib/ZStdOutputStream.h"

#include "shaback.h"
#include "RuntimeConfig.h"

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
      cerr << config.color_error << "Looks like a shaback repository already: " << config.repository << config.color_default << endl;
      exit(3);
    }
    if (!config.repoDir.listFiles("*").empty()) {
      cerr << "Destination directory is not empty: " << config.repository << endl;
      exit(3);
    }
  }

  if (!config.repoDir.isDir()) {
    if (!config.repoDir.mkdir()) {
      cerr << "Cannot create directory: " << config.repoDir.path << endl;
      exit(3);
    }
  }

  if (!config.filesDir.isDir()) {
    if (!config.filesDir.mkdir()) {
      cerr << "Cannot create directory: " << config.filesDir.path << endl;
      exit(3);
    }
  }
  if (!config.indexDir.isDir()) {
    if (!config.indexDir.mkdir()) {
      cerr << "Cannot create directory: " << config.indexDir.path << endl;
      exit(3);
    }
  }
  if (!config.locksDir.isDir()) {
    if (!config.locksDir.mkdir()) {
      cerr << "Cannot create directory: " << config.locksDir.path << endl;
      exit(3);
    }
  }
  if (!config.cacheDir.isDir()) {
    if (!config.cacheDir.mkdir()) {
      cerr << "Cannot create directory: " << config.cacheDir.path << endl;
      exit(3);
    }
  }

  char dirname[20];

  switch (config.init_repoFormat) {
    case REPOFORMAT_3:
      for (int level0 = 0; level0 <= 0xfff; level0++) {
        sprintf(dirname, "%03x", level0);
        File dirLevel0(config.filesDir, dirname);
        dirLevel0.mkdir();
      }
      break;

    default:
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
      break;
  }

  // Write default config:
  if (!config.repoPropertiesFile.isFile()) {
    string repoProperties =
        "# Don't modify this file!\n# Loss of data is inevitable!\n\n"
          "version = " SHABACK_REPO_VERSION "\n"
          "compression = ";
    repoProperties.append(Repository::compressionToName(config.init_compressionAlgorithm)).append("\nencryption = ") .append(
        Repository::encryptionToName(config.init_encryptionAlgorithm)).append("\n"
      "digest = SHA1\n");
    repoProperties.append("repoFormat = ").append(Repository::repoFormatToName(config.init_repoFormat)).append("\n");
    FileOutputStream os(config.repoPropertiesFile);
    os.write(repoProperties.data(), repoProperties.size());
  }

  if (config.init_encryptionAlgorithm != ENCRYPTION_NONE && !config.passwordCheckFile.isFile()) {
#if defined(OPENSSL_FOUND)
    // Create "password" file:
    string hash = Repository::hashPassword(config.init_encryptionAlgorithm, config.cryptoPassword);
    FileOutputStream os(config.passwordCheckFile);
    os.write(hash.data(), hash.size());
    os.close();
#else
    cerr << config.color_error << "Cannot handle encrypted repositories - missing openssl." << config.color_default << endl;
    exit(1);
#endif
  }

  cout << config.color_success << "Repository created: " << config.repository << config.color_default << endl;
}

int Shaback::deflate()
{
  StandardInputStream in(stdin);
  StandardOutputStream out(stdout);

  char buf[DEFLATE_CHUNK_SIZE];

  OutputStream* compressionOutputStream = ShabackOutputStream::createCompressionStream(&out, config.init_compressionAlgorithm);

  int bytesRead;

  while (true) {
    bytesRead = in.read(buf, DEFLATE_CHUNK_SIZE);
    if (bytesRead == -1)
      break;
    compressionOutputStream->write(buf, bytesRead);
  }

  compressionOutputStream->close();
  delete compressionOutputStream;

  return 0;
}

int Shaback::inflate()
{
  StandardInputStream in(stdin);
  StandardOutputStream out(stdout);

  char buf[DEFLATE_CHUNK_SIZE];

  InputStream* compressionInputStream = ShabackInputStream::createCompressionStream(&in, config.init_compressionAlgorithm);

  int bytesRead;

  while (true) {
    bytesRead = compressionInputStream->read(buf, DEFLATE_CHUNK_SIZE);
    if (bytesRead == -1)
      break;
    out.write(buf, bytesRead);
  }

  compressionInputStream->close();
  delete compressionInputStream;

  return 0;
}
