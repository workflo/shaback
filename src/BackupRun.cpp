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
#if defined(SHABACK_HAS_BACKUP)
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
    shaback_filesize_t totalSubDirSize = 0;

    try {
      if (file.isSymlink()) {
        rootFile.append(handleSymlink(file, true));
      } else if (file.isDir()) {
        rootFile.append(handleDirectory(file, true, &totalSubDirSize));
      } else if (file.isFile()) {
        rootFile.append(handleFile(file, true, &totalSubDirSize));
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

string BackupRun::handleDirectory(File& dir, bool absolutePaths, shaback_filesize_t* totalParentDirSize, bool skipChildren)
{
  string treeFile(TREEFILE_HEADER);
  treeFile.append("\n").append(dir.path).append("\n");
  shaback_filesize_t totalDirSize = 0;

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
              handleDirectory(child, false, &totalDirSize, (config.oneFileSystem && dir.getPosixDev() != child.getPosixDev())));
        } else if (child.isFile()) {
          treeFile.append(handleFile(child, false, &totalDirSize));
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
  sprintf(buf, "\t%03o\t%d\t%d\t%d\t%d\t%jd\t", dir.getPosixMode(), dir.getPosixUid(), dir.getPosixGid(),
      dir.getPosixMtime(), dir.getPosixCtime(), totalDirSize);
  treeFileLine.append(buf);
  treeFileLine.append("\n");

  config.runLeaveDirCallbacks(dir);

  *totalParentDirSize += totalDirSize;

  return treeFileLine;
}

string BackupRun::handleFile(File& file, bool absolutePaths, shaback_filesize_t* totalDirSize)
{
  shaback_filesize_t totalFileSize = 0;
  string hashValue = repository.storeFile(this, file, &totalFileSize);

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
      file.getPosixMtime(), file.getPosixCtime(), totalFileSize);
  treeFileLine.append(buf);
  treeFileLine.append("\n");

  *totalDirSize += totalFileSize;

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
}

void BackupRun::reportError(Exception& ex)
{
  numErrors++;
  cerr << "[E] " << ex.getMessage() << endl;
}

void BackupRun::deleteOldIndexFiles()
{
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
      cout << "Deleting old index file " << file.path.c_str() << endl;
    }
    file.remove();
  }
}
#endif
