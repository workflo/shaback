#ifndef BackupRun_H
#define BackupRun_H

#include <string>
#include "Repository.h"
#include "File.h"

class BackupRun
{
public:
    BackupRun(RuntimeConfig& config, Repository& Repository);
    ~BackupRun();
    
    int run();    

 protected:
/*    void walkFiles(File file); */
   std::string handleDirectory(File& dir, bool absolutePaths);
   std::string handleFile(File& dir, bool absolutePaths);
   std::string handleSymlink(File& dir, bool absolutePaths);
   void handleSymlink(File& dir);
   
   Repository& repository;
   RuntimeConfig& config;
};

#endif // BackupRun_H
