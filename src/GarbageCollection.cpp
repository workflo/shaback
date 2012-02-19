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

  repository.openWriteCache();
  repository.openReadCache();

  vector<File> rootFiles = config.indexDir.listFiles("*.sroot");

  for (vector<File>::iterator it = rootFiles.begin(); it < rootFiles.end(); it++) {
    File rootFile(*it);
    if (config.verbose)
      cout << "Reading root file: " << rootFile.path << "\r";
    processRootFile(rootFile);
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

void GarbageCollection::processRootFile(File& rootFile)
{
  FileInputStream in(rootFile);
  string hashValue;
  if (in.readLine(hashValue)) {
    repository.writeCache.put(hashValue);
    processTreeFile(hashValue);
  } else {
    throw GarbageCollectionException(string("Root index file is empty: ").append(rootFile.path));
  }
}

void GarbageCollection::processTreeFile(std::string id)
{
  try {
    vector<TreeFileEntry> entries = repository.loadTreeFile(id);
    for (vector<TreeFileEntry>::iterator it = entries.begin(); it < entries.end(); it++) {
      TreeFileEntry entry(*it);
      switch (entry.type) {
        case TREEFILEENTRY_DIRECTORY:
          repository.writeCache.put(entry.id);
          processTreeFile(entry.id);
          break;

        case TREEFILEENTRY_FILE:
          repository.writeCache.put(entry.id);
          if (entry.isSplitFile) {
            keepSplitFileBlocks(entry);
          }
          break;

        case TREEFILEENTRY_SYMLINK:
          // Do nothing.
          break;
      }
    }
  } catch (Exception& ex) {
    reportError(ex);
  }
}

void GarbageCollection::keepSplitFileBlocks(TreeFileEntry& entry)
{
  cout << "keepSplitFileBlocks for " << entry.path << endl;
  SplitFileIndexReader reader(repository, entry.id);
  string hashValue;

  while (reader.next(hashValue)) {
    repository.writeCache.put(hashValue);
  }
}

void GarbageCollection::reportError(Exception& ex)
{
  numErrors++;
  cerr << "[E] " << ex.getMessage() << endl;
}

void GarbageCollection::showTotals()
{
  printf("Files deleted:     %12d\n", filesDeleted);
  printf("Tmp files deleted: %12d\n", tmpFilesDeleted);
  printf("Errors:            %12d\n", numErrors);
}

void GarbageCollection::removeUnusedFiles()
{
  char dirname[3];
  char idPrefix[5];

  for (int level0 = 0; level0 <= 0xff; level0++) {
    sprintf(dirname, "%02x", level0);
    File dirLevel0(config.filesDir, dirname);

    if (config.verbose)
      cout << "Removing garbage from: " << dirLevel0.path << "\r";

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
        } else if (!repository.writeCache.contains(id)) {
          // Delete unreferenced files:
          if (f.remove()) {
            if (config.debug)
              cout << "[d] " << f.path << endl;
            filesDeleted++;
            repository.readCache.remove(id);
          }
        }
      }
    }
  }
  if (config.verbose)
    cout << endl;
}
