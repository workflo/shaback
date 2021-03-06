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
#include <string.h>

#include "lib/Exception.h"
#include "lib/FileInputStream.h"
#include "lib/FileOutputStream.h"

#include "GarbageCollection.h"
#include "ShabackException.h"
#include "SplitFileIndexReader.h"
#include "DirectoryFileReader.h"

using namespace std;

GarbageCollection::GarbageCollection(RuntimeConfig& config, Repository& repository) :
    repository(repository), config(config), numErrors(0), tmpFilesDeleted(0), filesDeleted(0)
{
}

GarbageCollection::~GarbageCollection()
{
  repository.unlock();
}

void GarbageCollection::run()
{
  repository.lock(true);
  repository.open();

  vector<File> directoryFiles = config.indexDir.listFiles("*.shabackup");

  for (vector<File>::iterator it = directoryFiles.begin(); it < directoryFiles.end(); it++) {
    File shabackupFile(*it);
    if (config.verbose)
      cout << "Reading directory file: " << shabackupFile.path << endl;
    processShabackupFile(shabackupFile);
  }

  if (config.verbose)
    cout << endl;

  if (numErrors > 0 && !config.force)
    throw GarbageCollectionException(
        "There where errors. Bailing out, no harm has been done to archive! Use '--force' to override.");

  repository.removeAllCacheFiles();
  removeUnusedFiles();
  repository.exportCacheFile();

  showTotals();
}

void GarbageCollection::processShabackupFile(File& shabackupFile)
{
  DirectoryFileReader dirFileReader(repository, shabackupFile);
  dirFileReader.open();
  
  try {
    do {
      TreeFileEntry entry(dirFileReader.next());
      if (entry.isEof()) break;

      switch (entry.type) {
        case TREEFILEENTRY_FILE:
          repository.writeCache.insert(entry.id);
          if (entry.isSplitFile) {
            keepSplitFileBlocks(entry);
          }
          break;

        default:
          // Do nothing.
          break;
      }
    } while(true);
  } catch (Exception& ex) {
    reportError(ex);
  }
}

void GarbageCollection::keepSplitFileBlocks(TreeFileEntry& entry)
{
  SplitFileIndexReader reader(repository, entry.id);
  string hashValue;

  while (reader.next(hashValue)) {
    repository.writeCache.insert(hashValue);
  }
}

void GarbageCollection::reportError(Exception& ex)
{
  numErrors++;
  cerr << config.color_error << "[E] " << ex.getMessage() << config.color_default << endl;
}

void GarbageCollection::showTotals()
{
  cerr << (numErrors == 0 ? config.color_stats : config.color_error);
  printf("Files deleted:     %12d\n", filesDeleted);
  printf("Tmp files deleted: %12d\n", tmpFilesDeleted);
  printf("Errors:            %12d\n", numErrors);
  cout << config.color_default;
}

void GarbageCollection::removeUnusedFiles()
{
  char dirname[4];
  char idPrefix[5];

  switch (repository.repoFormat) {
    case REPOFORMAT_3:
      for (int level0 = 0; level0 <= 0xfff; level0++) {
        sprintf(dirname, "%03x", level0);
        File dirLevel0(config.filesDir, dirname);

        if (config.verbose)
          cout << "Removing garbage from: " << dirLevel0.path << "\r" << flush;

        vector<File> tmpFiles = dirLevel0.listFiles("*");
        for (vector<File>::iterator it = tmpFiles.begin(); it < tmpFiles.end(); it++) {
          File f(*it);

          string id = dirname;
          id.append(f.fname);

          if (f.fname.find(".tmp") != string::npos) {
            // Delete all tmp files:
            if (f.remove()) {
              if (config.debug)
                cout << "[d] " << f.path << endl;
              tmpFilesDeleted++;
            }
          } else if (!repository.writeCache.count(id)) {
            // Delete unreferenced files:
            if (f.remove()) {
              if (config.debug)
                cout << "[d] " << f.path << endl;
              filesDeleted++;
            }
          }
        }
      }
      break;

    default:
      for (int level0 = 0; level0 <= 0xff; level0++) {
        sprintf(dirname, "%02x", level0);
        File dirLevel0(config.filesDir, dirname);

        if (config.verbose)
          cout << "Removing garbage from: " << dirLevel0.path << "\r" << flush;

        for (int level1 = 0; level1 <= 0xff; level1++) {
          sprintf(dirname, "%02x", level1);
          sprintf(idPrefix, "%02x%02x", level0, level1);
          File dirLevel1(dirLevel0, dirname);

          vector<File> tmpFiles = dirLevel1.listFiles("*");
          for (vector<File>::iterator it = tmpFiles.begin(); it < tmpFiles.end(); it++) {
            File f(*it);

            string id = idPrefix;
            id.append(f.fname);

            if (f.fname.find(".tmp") != string::npos) {
              // Delete all tmp files:
              if (f.remove()) {
                if (config.debug)
                  cout << "[d] " << f.path << endl;
                tmpFilesDeleted++;
              }
            } else if (!repository.writeCache.count(id)) {
              // Delete unreferenced files:
              if (f.remove()) {
                if (config.debug)
                  cout << "[d] " << f.path << endl;
                filesDeleted++;
              }
            }
          }
        }
      }
  }

  if (config.verbose)
    cout << endl;
}
