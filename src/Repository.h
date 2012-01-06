#ifndef SHABACK_Repository_H
#define SHABACK_Repository_H

#include <string>
#include <vector>
#include "RuntimeConfig.h"
#include "Cache.h"
#include "lib/Date.h"
#include "TreeFileEntry.h"

class BackupRun;

class Repository
{
  public:
    Repository(RuntimeConfig& config);
    ~Repository();

    int backup();
    void restore();

    void open();
    void lock();
    void unlock();

    File hashValueToFile(std::string hashValue);
    bool contains(std::string& hashValue);
    std::string storeTreeFile(BackupRun* run, std::string& treeFile);
    std::string storeFile(BackupRun* run, File& srcFile);
    void storeRootTreeFile(std::string& rootHashValue);
    void importCacheFile();
    void exportCacheFile();
    std::vector<TreeFileEntry> loadTreeFile(std::string& treeId);
    void exportFile(TreeFileEntry& entry, File& outFile);
    void exportSymlink(TreeFileEntry& entry, File& outFile);

  protected:
    RuntimeConfig config;
    int hashAlgorithm;
    int encryptionAlgorithm;
    int compressionAlgorithm;
    Cache cache;
    Date startDate;

    void openCache();
    void restoreByRootFile(File& rootFile);
    void restoreByTreeId(std::string& treeId);
};

#define COMPRESSION_NONE    0
#define COMPRESSION_DEFLATE 1

#define ENCRYPTION_NONE     0
#define ENCRYPTION_BLOWFISH 1
#define ENCRYPTION_AES      2
#define ENCRYPTION_DES      3

#define DIGEST_SHA1         1
#define DIGEST_SHA256       2

#endif // SHABACK_Repository_H
