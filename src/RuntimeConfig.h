#ifndef SHABACK_RuntimeConfig_H
#define SHABACK_RuntimeConfig_H

#include <string>
#include <vector>
#include "lib/File.h"

extern "C" {
# include <lua.h>
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
    bool help;

    std::string operation;
    std::string repository;
    std::string localCacheFile;
    std::string backupName;
    lua_State* luaState;
    std::vector<std::string> excludePatterns;
    std::vector<std::string> splitPatterns;
    std::vector<std::string> dirs;
    std::vector<std::string> cliArgs;
    std::string cryptoPassword;
    
    File filesDir;
    File indexDir;
    File locksDir;
    File cacheDir;
    File repoDir;
    File repoPropertiesFile;
    
    bool excludeFile(File& file);
    
    void finalize();

 protected:
    void initLua();
    void tryToLoadFrom(std::string dir);
};

#endif // SHABACK_RuntimeConfig_H
