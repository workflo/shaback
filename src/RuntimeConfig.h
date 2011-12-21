#ifndef RuntimeConfig_H
#define RuntimeConfig_H

#include <string>
#include <vector>
#include "File.h"

extern "C" {
#include <lua.h>
}

class RuntimeConfig
{
public:
    RuntimeConfig();
    virtual void load();
    virtual void parseCommandlineArgs(int argc, char** argv);
    virtual void loadConfigFile(std::string filename);

    bool verbose;
    bool force;
    bool oneFileSystem;
    std::string operation;
    std::string repository;
    std::string localCacheFile;
    lua_State* luaState;
    std::vector<std::string> excludePatterns;
    std::vector<std::string> splitPatterns;
    std::vector<std::string> dirs;
    
    File filesDir;
    File indexDir;
    File locksDir;
    
    bool excludeFile(File& file);
    
    void finalize();

 protected:
    void initLua();
    void tryToLoadFrom(std::string dir);
};

#endif // RuntimeConfig_H
