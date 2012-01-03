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
    bool debug;
    bool force;
    bool oneFileSystem;
    bool showTotals;

    std::string operation;
    std::string repository;
    std::string localCacheFile;
    lua_State* luaState;
    std::vector<std::string> excludePatterns;
    std::vector<std::string> splitPatterns;
    std::vector<std::string> dirs;
    std::string cryptoPassword;
    
    File filesDir;
    File indexDir;
    File locksDir;
    File cacheDir;
    
    bool excludeFile(File& file);
    
    void finalize();

 protected:
    void initLua();
    void tryToLoadFrom(std::string dir);
};

#endif // RuntimeConfig_H
