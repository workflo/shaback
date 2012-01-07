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
  repository(repository), config(config), numErrors(0)
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
      cout << rootFile.path << endl;
    processRootFile(rootFile);
  }

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
  printf("Errors:           %12d\n", numErrors);
}
