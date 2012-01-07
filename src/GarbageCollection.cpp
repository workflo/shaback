#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lib/Exception.h"
#include "lib/FileInputStream.h"
#include "lib/FileOutputStream.h"

#include "GarbageCollection.h"
#include "ShabackException.h"

using namespace std;

GarbageCollection::GarbageCollection(RuntimeConfig& config, Repository& repository) :
  repository(repository), config(config), numErrors(0), tmpFilesDeleted(0), filesDeleted(0)
{
  repository.lock(true);
}

GarbageCollection::~GarbageCollection()
{
  repository.unlock();
}

void GarbageCollection::run()
{
  repository.open();
  if (config.localCacheFile.empty()) {
    throw GarbageCollectionException("A local cache file is needed for garbage collection.");
  } else {
    repository.openCache();
  }

  vector<File> rootFiles = config.indexDir.listFiles("*.sroot");

  for (vector<File>::iterator it = rootFiles.begin(); it < rootFiles.end(); it++) {
    File rootFile(*it);
    if (config.verbose)
      cout << "reading root file: " << rootFile.path << endl;
    processRootFile(rootFile);
  }

  if (numErrors > 0 && !config.force)
    throw GarbageCollectionException(
        "There where errors. Bailing out, no harm has been done to archive! Use '--force' to override.");

  removeUnusedFiles();

  showTotals();
}

void GarbageCollection::processRootFile(File& rootFile)
{
  FileInputStream in(rootFile);
  string hashValue;
  if (in.readLine(hashValue)) {
    repository.cache.put(hashValue);
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
          repository.cache.put(entry.id);
          processTreeFile(entry.id);
          break;

        case TREEFILEENTRY_FILE:
          repository.cache.put(entry.id);
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

    for (int level1 = 0; level1 <= 0xff; level1++) {
      sprintf(dirname, "%02x", level1);
      sprintf(idPrefix, "%02x%02x", level0, level1);
      File dirLevel1(dirLevel0, dirname);

      if (config.verbose)
        cout << dirLevel1.path << endl;

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
        } else if (!repository.cache.contains(id)) {
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
