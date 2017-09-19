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

  for (vector<string>::iterator it = config.dirs.begin(); it < config.dirs.end(); it++) {
    File file(*it);
    intmax_t totalSubDirSize = 0;

    try {
      if (file.isSymlink()) {
        handleSymlink(file, true);
      } else if (file.isDir()) {
        handleDirectory(file, true, &totalSubDirSize);
      } else if (file.isFile()) {
        handleFile(file, true, &totalSubDirSize);
      } else if (!file.exists()) {
        throw FileNotFoundException(file.path);
      } else {
        // Ignore other types of files.
      }
    } catch (Exception& ex) {
      reportError(ex);
    }
  }

  directoryFileStream.close();

  // string rootFileHashValue = repository.storeTreeFile(this, rootFile);

  // repository.storeRootTreeFile(rootFileHashValue);

  deleteOldIndexFiles();

  return (numErrors == 0 ? 0 : 1);
}


void BackupRun::openDirectoryFile()
{
  // FIXME: Race condition! Add random part to filename.
  string filename = config.backupName;
  filename.append("_").append(repository.startDate.toFilename()).append(".shaback-backup");

  directoryFileStream.open(File(config.indexDir, filename));

  directoryFileStream.write(DIRECTORY_FILE_HEADER);
  directoryFileStream.write("\n");
}


string BackupRun::handleDirectory(File& dir, bool absolutePaths, intmax_t* totalParentDirSize, bool skipChildren)
{
  intmax_t totalDirSize = 0;

  config.runEnterDirCallbacks(dir);

  if (!skipChildren) {
    vector<File> files = dir.listFiles("*");

    for (vector<File>::iterator it = files.begin(); it < files.end(); it++) {
      File child(*it);

      if (config.excludeFile(child))
        continue;

      try {
        if (child.isSymlink()) {
          handleSymlink(child, false);
        } else if (child.isDir()) {
          handleDirectory(child, false, &totalDirSize, (config.oneFileSystem && dir.getPosixDev() != child.getPosixDev()));
        } else if (child.isFile()) {
          handleFile(child, false, &totalDirSize);
        } else {
          // Ignore other types of files.
        }
      } catch (Exception& ex) {
        reportError(ex);
      }
    }
  }

  char buf[100];
  sprintf(buf, "\t%03o\t%d\t%d\t%d\t%d\t%jd\t\n", dir.getPosixMode(), dir.getPosixUid(), dir.getPosixGid(),
      dir.getPosixMtime(), dir.getPosixCtime(), totalDirSize);

  directoryFileStream.write("D\t\t");
  directoryFileStream.write(dir.path);
  directoryFileStream.write(buf);
  
  config.runLeaveDirCallbacks(dir);

  *totalParentDirSize += totalDirSize;

  return "";
}

string BackupRun::handleFile(File& file, bool absolutePaths, intmax_t* totalDirSize)
{
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

  *totalDirSize += totalFileSize;

  directoryFileStream.write(treeFileLine);

  return treeFileLine;
}

string BackupRun::handleSymlink(File& file, bool absolutePaths)
{
  string treeFileLine("S\t*\t");
  treeFileLine.append(file.path);

  char buf[100];
  sprintf(buf, "\t%03o\t%d\t%d\t%d\t%d\t\t", file.getPosixMode(), file.getPosixUid(), file.getPosixGid(),
      file.getPosixMtime(), file.getPosixCtime());
  treeFileLine.append(buf);
  treeFileLine.append(file.readlink());
  treeFileLine.append("\n");

  directoryFileStream.write(treeFileLine);
  
  return treeFileLine;
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

void BackupRun::deleteOldIndexFiles()
{
  // TODO: Move to new class History!
  string pattern(config.backupName);
  pattern.append("_????" "-??" "-??_??????.sroot");

  vector<File> indexFiles = config.indexDir.listFiles(pattern);

  if (indexFiles.size() <= 2) return; // Don't delete latest two backups

  sort(indexFiles.begin(), indexFiles.end(), filePathComparator);
  reverse(indexFiles.begin(), indexFiles.end());
  vector<Date> dates;
  vector<Date> toDelete;

  for (vector<File>::iterator it = indexFiles.begin(); it < indexFiles.end(); it++) {
    File file(*it);
    Date d(file.fname.substr(config.backupName.size() + 1));
    dates.push_back(d);
  }

  int idx = 0;
  Date now;

  Date upper(now);
  upper.addDays(-config.keepOldBackupsBoundaries[0]);
  upper.setTimeOfDay(0, 0, 0);

//  cout << "   upper: " << upper.toFilename() << endl;

  // Keep daily backups:
  Date dailyLimit(upper);
  dailyLimit.addDays(-config.keepOldBackupsBoundaries[1]);
  while (true) {
    Date lower(upper);
    lower.addDays(-1);
    if (lower.compareTo(dailyLimit) <= 0)
      break;
//    cout << "   deleting " << lower.toFilename() << " .. " << upper.toFilename() << endl;
    bool found = false;
    for (vector<Date>::iterator it = dates.begin(); it < dates.end(); it++) {
      Date d(*it);
      if (d.compareTo(lower) >= 0 && d.compareTo(upper) < 0) {
        toDelete.push_back(d);
        found = true;
      }
    }
    upper = lower;

    // Keep last one from this range:
    if (found) toDelete.pop_back();
  }

  // Keep weekly backups:
  Date weeklyLimit(upper);
  weeklyLimit.addDays(-config.keepOldBackupsBoundaries[2]);
  while (true) {
    Date lower(upper);
    lower.addDays(-7);
    if (lower.compareTo(weeklyLimit) <= 0)
      break;
    bool found = false;
//    cout << "   deleting " << lower.toFilename() << " .. " << upper.toFilename() << endl;
    for (vector<Date>::iterator it = dates.begin(); it < dates.end(); it++) {
      Date d(*it);
      if (d.compareTo(lower) >= 0 && d.compareTo(upper) < 0) {
        toDelete.push_back(d);
        found = true;
      }
    }
    upper = lower;

    // Keep last one from this range:
    if (found) toDelete.pop_back();
  }

  // Keep monthly backups:
  Date monthlyLimit(dates.back());
  while (true) {
    Date lower(upper);
    lower.addDays(-30);
    bool found = false;
//    cout << "   deleting " << lower.toFilename() << " .. " << upper.toFilename() << endl;
    for (vector<Date>::iterator it = dates.begin(); it < dates.end(); it++) {
      Date d(*it);
      if (d.compareTo(lower) >= 0 && d.compareTo(upper) < 0) {
        toDelete.push_back(d);
        found = true;
      }
    }
    upper = lower;

    // Keep last one from this range:
    if (found) toDelete.pop_back();

    if (lower.compareTo(monthlyLimit) <= 0)
      break;
  }

  // Actually delete files:
  for (vector<Date>::iterator it = toDelete.begin(); it < toDelete.end(); it++) {
    Date d(*it);
    string fname(config.backupName);
    fname.append("_").append(d.toFilename()).append(".sroot");
    File file(config.indexDir, fname);
    if (config.verbose) {
      cout << config.color_deleted << "Deleting old index file " << file.path.c_str() << config.color_default << endl;
    }
    file.remove();
  }
}
