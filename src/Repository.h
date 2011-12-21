#ifndef Repository_H
#define Repository_H

#include <string>
#include "RuntimeConfig.h"
#include "Cache.h"

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
    std::string storeTreeFile(std::string& treeFile);
    std::string storeFile(File& srcFile);

 protected:
   RuntimeConfig config;
   int hashAlgorithm;
   int encryptionAlgorithm;
   int compressionAlgorithm;
   Cache cache;
};


#define COMPRESSION_NONE    0
#define COMPRESSION_GZ      1
#define COMPRESSION_LZO     2

#define ENCRYPTION_NONE     0
#define ENCRYPTION_AES      1
#define ENCRYPTION_DES      2

#define DIGEST_SHA1         1
#define DIGEST_SHA256       2

#endif // Repository_H
