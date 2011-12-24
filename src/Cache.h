#ifndef SHABACK_Cache_H
#define SHABACK_Cache_H

#include <string>
#include "File.h"

extern "C" {
#include <gdbm.h>
}

class Cache
{
  public:
    Cache(File file);
    ~Cache();
    
    void open();
    void close();
    bool contains(std::string& key);
    void put(std::string& key, std::string& value);
    void put(std::string& key);
    void remove(std::string& key);
    
  private:
    File file;
    GDBM_FILE gdbmFile;
    bool opened;
};

#endif // SHABACK_Cache_H