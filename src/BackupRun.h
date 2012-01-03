#ifndef BackupRun_H
#define BackupRun_H

#include <string>
#include "Repository.h"
#include "File.h"
#include "Exception.h"

class BackupRun
{
  public:
    BackupRun(RuntimeConfig& config, Repository& Repository);
    ~BackupRun();

    int run();
    void showTotals();
    void reportError(Exception& ex);

    off_t numBytesRead;
    off_t numBytesStored;
    int numFilesRead;
    int numFilesStored;
    int numErrors;

  protected:
    std::string handleDirectory(File& dir, bool absolutePaths);
    std::string handleFile(File& dir, bool absolutePaths);
    std::string handleSymlink(File& dir, bool absolutePaths);
    void handleSymlink(File& dir);

    Repository& repository;
    RuntimeConfig& config;
};

#endif // BackupRun_H
