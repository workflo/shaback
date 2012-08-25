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
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "BackupRun.h"
#include "lib/Sha1.h"
#include "lib/Exception.h"
#include "TreeFile.h"
#include "TreeFileEntry.h"

using namespace std;

BackupRun::BackupRun(RuntimeConfig& config, Repository& repository) :
    repository(repository), config(config)
{
  numFilesRead = 0;
  numFilesStored = 0;
  numBytesRead = 0;
  numBytesStored = 0;
  numErrors = 0;
}

BackupRun::~BackupRun()
{
  repository.unlock();
}

int BackupRun::run()
{
  repository.lock();

  if (!config.cliArgs.empty()) {
    // Override backup set from files specified on the command line:
    config.dirs = config.cliArgs;
  } else if (config.dirs.empty()) {
    cerr << "No files to backup." << endl;
    return 5;
  }

  config.runPreBackupCallbacks();

  string rootFile(TREEFILE_HEADER);
  rootFile.append("\n\n");

  for (vector<string>::iterator it = config.dirs.begin(); it < config.dirs.end(); it++) {
    File file(*it);

    try {
      if (file.isSymlink()) {
        rootFile.append(handleSymlink(file, true));
      } else if (file.isDir()) {
        rootFile.append(handleDirectory(file, true));
      } else if (file.isFile()) {
        rootFile.append(handleFile(file, true));
      } else if (!file.exists()) {
        throw FileNotFoundException(file.path);
      } else {
        // Ignore other types of files.
      }
    } catch (Exception& ex) {
      reportError(ex);
    }
  }

  string rootFileHashValue = repository.storeTreeFile(this, rootFile);

  repository.storeRootTreeFile(rootFileHashValue);

  deleteOldIndexFiles();

  return (numErrors == 0 ? 0 : 1);
}

string BackupRun::handleDirectory(File& dir, bool absolutePaths, bool skipChildren)
{
  string treeFile(TREEFILE_HEADER);
  treeFile.append("\n").append(dir.path).append("\n");

  config.runEnterDirCallbacks(dir);

  if (!skipChildren) {
    vector<File> files = dir.listFiles("*");

    for (vector<File>::iterator it = files.begin(); it < files.end(); it++) {
      File child(*it);

      if (config.excludeFile(child))
        continue;

      try {
        if (child.isSymlink()) {
          treeFile.append(handleSymlink(child, false));
        } else if (child.isDir()) {
          treeFile.append(
              handleDirectory(child, false, (config.oneFileSystem && dir.getPosixDev() != child.getPosixDev())));
        } else if (child.isFile()) {
          treeFile.append(handleFile(child, false));
        } else {
          // Ignore other types of files.
        }
      } catch (Exception& ex) {
        reportError(ex);
      }
    }
  }

  string treeFileHashValue = repository.storeTreeFile(this, treeFile);

  string treeFileLine("D\t");
  treeFileLine.append(treeFileHashValue);
  treeFileLine.append("\t");
  if (absolutePaths) {
    treeFileLine.append(dir.path);
  } else {
    treeFileLine.append(dir.fname);
  }

  char buf[100];
  sprintf(buf, "\t%03o\t%d\t%d\t%d\t%d\t\t", dir.getPosixMode(), dir.getPosixUid(), dir.getPosixGid(),
      dir.getPosixMtime(), dir.getPosixCtime());
  treeFileLine.append(buf);
  treeFileLine.append("\n");

  config.runLeaveDirCallbacks(dir);

  return treeFileLine;
}

string BackupRun::handleFile(File& file, bool absolutePaths)
{
  string hashValue = repository.storeFile(this, file);

  string treeFileLine("F\t");
  treeFileLine.append(hashValue);
  treeFileLine.append("\t");
  if (absolutePaths) {
    treeFileLine.append(file.path);
  } else {
    treeFileLine.append(file.fname);
  }

  char buf[100];
  sprintf(buf, "\t%03o\t%d\t%d\t%d\t%d\t%jd\t", file.getPosixMode(), file.getPosixUid(), file.getPosixGid(),
      file.getPosixMtime(), file.getPosixCtime(),
#ifdef __APPLE__
      (intmax_t) file.getSize()
#else
      file.getSize()
#endif
      );
  treeFileLine.append(buf);
  treeFileLine.append("\n");

  return treeFileLine;
}

string BackupRun::handleSymlink(File& file, bool absolutePaths)
{
  string treeFileLine("S\t*\t");
  if (absolutePaths) {
    treeFileLine.append(file.path);
  } else {
    treeFileLine.append(file.fname);
  }

  char buf[100];
  sprintf(buf, "\t%03o\t%d\t%d\t%d\t%d\t\t", file.getPosixMode(), file.getPosixUid(), file.getPosixGid(),
      file.getPosixMtime(), file.getPosixCtime());
  treeFileLine.append(buf);
  treeFileLine.append(file.readlink());
  treeFileLine.append("\n");

  return treeFileLine;
}

void BackupRun::showTotals()
{
  printf("Files inspected:  %12d\n", numFilesRead);
#ifdef __APPLE__
  printf("Bytes inspected:  %12jd\n", (intmax_t) numBytesRead);
#else
  printf("Bytes inspected:  %12lu\n", numBytesRead);
#endif
  printf("Files stored:     %12d\n", numFilesStored);
#ifdef __APPLE__
  printf("Bytes stored:     %12jd\n", (intmax_t) numBytesStored);
#else
  printf("Bytes stored:     %12lu\n", numBytesStored);
#endif
  printf("Errors:           %12d\n", numErrors);
}

void BackupRun::reportError(Exception& ex)
{
  numErrors++;
  cerr << "[E] " << ex.getMessage() << endl;
}

void BackupRun::deleteOldIndexFiles()
{
  string pattern(config.backupName);
  pattern.append("_????""-??""-??_??????.sroot");

  vector<File> indexFiles = config.indexDir.listFiles(pattern);
  sort(indexFiles.begin(), indexFiles.end(), filePathComparator);
  reverse(indexFiles.begin(), indexFiles.end());

  for (vector<File>::iterator it = indexFiles.begin(); it < indexFiles.end(); it++) {
    File file(*it);
    cout << file.path << endl;
  }
}
