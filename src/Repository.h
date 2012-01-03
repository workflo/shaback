#ifndef Repository_H
#define Repository_H

#include <string>
#include "RuntimeConfig.h"
#include "Cache.h"

class BackupRun;

class Repository
{
public:
    Repository(RuntimeConfig& config);
    ~Repository();
    
    int backup();
    
    void open();
    void lock();
    void unlock();

    File hashValueToFile(std::string hashValue);
    bool contains(std::string& hashValue);
    std::string storeTreeFile(BackupRun* run, std::string& treeFile);
    std::string storeFile(BackupRun* run, File& srcFile);
    void importCacheFile();
    void exportCacheFile();

 protected:
   RuntimeConfig config;
   int hashAlgorithm;
   int encryptionAlgorithm;
   int compressionAlgorithm;
   Cache cache;
};


#define COMPRESSION_NONE    0
#define COMPRESSION_DEFLATE 1

#define ENCRYPTION_NONE     0
#define ENCRYPTION_BLOWFISH 1
#define ENCRYPTION_AES      2
#define ENCRYPTION_DES      3

#define DIGEST_SHA1         1
#define DIGEST_SHA256       2

#endif // Repository_H
