#ifndef SHABACK_GarbageCollection_H
#define SHABACK_GarbageCollection_H

#include <string>
#include "Repository.h"
#include "lib/File.h"
#include "lib/Exception.h"

class GarbageCollection
{
  public:
    GarbageCollection(RuntimeConfig& config, Repository& Repository);
    ~GarbageCollection();

    void run();
    void showTotals();

  protected:
    void processRootFile(File& rootFile);
    void processTreeFile(std::string id);
    void reportError(Exception& ex);
    void removeUnusedFiles();

    Repository& repository;
    RuntimeConfig& config;
    int numErrors;
    int tmpFilesDeleted;
    int filesDeleted;
};

#endif // SHABACK_GarbageCollection_H