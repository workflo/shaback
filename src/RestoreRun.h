#ifndef SHABACK_RestoreRun_H
#define SHABACK_RestoreRun_H

#include <string>
#include "Repository.h"

class RestoreRun
{
  public:
    RestoreRun(RuntimeConfig& config, Repository& Repository);
    ~RestoreRun();

    void restore(std::string& treeId, File& destinationDir);

    off_t numBytesRestored;
    int numFilesRestored;
    int numErrors;

  private:
    Repository& repository;
    RuntimeConfig& config;
};

#endif // SHABACK_RestoreRun_H
