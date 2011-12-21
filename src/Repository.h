#ifndef Repository_H
#define Repository_H

#include <string>
#include "RuntimeConfig.h"
#include "GDBM.h"

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
   std::string hashAlgorithm;
   std::string cypherAlgorithm;
   std::string compressionAlgorithm;
   GDBM cache;
};

#endif // Repository_H
