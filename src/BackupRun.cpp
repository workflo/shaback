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
  repository(repository), config(config),
  directoryFileStream(repository.createOutputStream())
{
  numFilesRead = 0;
  numFilesStored = 0;
  numBytesRead = 0;
  numBytesStored = 0;
  numErrors = 0;
}

BackupRun::~BackupRun()
{
  directoryFileStream.close();
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

  openDirectoryFile();

  for (list<string>::iterator it = config.dirs.begin(); it != config.dirs.end(); it++) {
    File file(*it);
    intmax_t totalSubDirSize = 0;
    try {
      if (file.isSymlink()) {
        handleSymlink(file);
      } else if (file.isDir()) {
        handleDirectory(file, false);
      } else if (file.isFile()) {
        handleFile(file);
      } else if (!file.exists()) {
        throw FileNotFoundException(file.path);
      } else {
        // Ignore other types of files.
      }
    } catch (Exception& ex) {
      reportError(ex);
    }
  }

  directoryFileStream.finish();

  repository.deleteOldIndexFiles(config.backupName);

  return (numErrors == 0 ? 0 : 1);
}


void BackupRun::openDirectoryFile()
{
  string filename = config.backupName;
  filename.append("_").append(repository.startDate.toFilename()).append(".shabackup");

  directoryFileStream.open(File(config.indexDir, filename));

  directoryFileStream.write(DIRECTORY_FILE_HEADER);
  directoryFileStream.write("\n");
}


bool fileNameOk(File file)
{
  const char* p = file.path.c_str();
  for ( ; *p != 0; p++) {
    if (*p == '\n' || *p == '\t' || *p == '\r') return false;
  }
  return true;
}

void BackupRun::handleDirectory(File& dir, bool skipChildren)
{
  if (!fileNameOk(dir.path)) {
    if (!config.ignoreErrors.count("invalid-filename")) {
      reportError(string("Ignoring directory name with control characters: \"").append(dir.path).append("\""));
    }
    return;
  }

  config.runEnterDirCallbacks(dir);

  char buf[100];
  sprintf(buf, "\t%03o\t%d\t%d\t%d\t%d\t%i\t\n", dir.getPosixMode(), dir.getPosixUid(), dir.getPosixGid(),
      dir.getPosixMtime(), dir.getPosixCtime(), 0);

  directoryFileStream.write("D\t\t");
  directoryFileStream.write(dir.path);
  directoryFileStream.write(buf);
  
  if (!skipChildren) {
    vector<File> files = dir.listFiles("*");

    for (vector<File>::iterator it = files.begin(); it < files.end(); it++) {
      File child(*it);

      if (config.excludeFile(child))
        continue;

      try {
        if (child.isSymlink()) {
          handleSymlink(child);
        } else if (child.isDir()) {
          handleDirectory(child, (config.oneFileSystem && dir.getPosixDev() != child.getPosixDev()));
        } else if (child.isFile()) {
          handleFile(child);
        } else {
          // Ignore other types of files.
        }
      } catch (Exception& ex) {
        reportError(ex);
      }
    }
  }

  config.runLeaveDirCallbacks(dir);
}

void BackupRun::handleFile(File& file)
{
  if (!fileNameOk(file.path)) {
    if (!config.ignoreErrors.count("invalid-filename")) {
      reportError(string("Ignoring filename with control characters: \"").append(file.path).append("\""));
    }
    return;
  }
  intmax_t totalFileSize = 0;
  string hashValue = repository.storeFile(this, file, &totalFileSize);

  string treeFileLine("F\t");
  treeFileLine.append(hashValue);
  treeFileLine.append("\t");
  treeFileLine.append(file.path);

  char buf[100];
  sprintf(buf, "\t%03o\t%d\t%d\t%d\t%d\t%jd\t", file.getPosixMode(), file.getPosixUid(), file.getPosixGid(),
      file.getPosixMtime(), file.getPosixCtime(), totalFileSize);
  treeFileLine.append(buf);
  treeFileLine.append("\n");

  directoryFileStream.write(treeFileLine);
}

void BackupRun::handleSymlink(File& file)
{
  if (!fileNameOk(file.path)) {
    if (!config.ignoreErrors.count("invalid-filename")) {
      reportError(string("Ignoring filename with control characters: \"").append(file.path).append("\""));
    }
    return;
  }

  string treeFileLine("S\t*\t");
  treeFileLine.append(file.path);

  char buf[100];
  sprintf(buf, "\t%03o\t%d\t%d\t%d\t%d\t\t", file.getPosixMode(), file.getPosixUid(), file.getPosixGid(),
      file.getPosixMtime(), file.getPosixCtime());
  treeFileLine.append(buf);
  treeFileLine.append(file.readlink());
  treeFileLine.append("\n");

  directoryFileStream.write(treeFileLine);
}

void BackupRun::showTotals()
{
  cerr << (numErrors == 0 ? config.color_stats : config.color_error);
  cerr << config.style_bold;
  fprintf(stderr, "Files inspected:  %12d\n", numFilesRead);
#ifdef __APPLE__
  fprintf(stderr, "Bytes inspected:  %12jd\n", numBytesRead);
#else
  fprintf(stderr, "Bytes inspected:  %12lu\n", numBytesRead);
#endif
  fprintf(stderr, "Files stored:     %12d\n", numFilesStored);
#ifdef __APPLE__
  fprintf(stderr, "Bytes stored:     %12jd\n", numBytesStored);
#else
  fprintf(stderr, "Bytes stored:     %12lu\n", numBytesStored);
#endif
  fprintf(stderr, "Errors:           %12d\n", numErrors);

  cerr << config.style_default;
}

void BackupRun::reportError(Exception& ex)
{
  numErrors++;
  cerr << config.color_error << "[E] " << ex.getMessage() << config.color_default << endl;
}

void BackupRun::reportError(string message)
{
  numErrors++;
  cerr << config.color_error << "[E] " << message << config.color_default << endl;
}
