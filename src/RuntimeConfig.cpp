#include <iostream>
//#include <unistd.h>
#ifdef WIN32
# include "getopt.h"
#else
# include <getopt.h>
# include <fnmatch.h>
#endif
#include <stdlib.h>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "RuntimeConfig.h"

#define LUA_RUNTIMECONFIG "__RuntimeConfig__"

using namespace std;

RuntimeConfig::RuntimeConfig()
{
  this->verbose = false;
  this->oneFileSystem = false;
  this->initLua();
}


void RuntimeConfig::parseCommandlineArgs(int argc, char** argv)
{
  int digit_optind = 0;

    while (true) {
	int this_option_optind = optind ? optind : 1;
	int option_index = 0;
	static struct option long_options[] = {
	    {"add", 1, 0, 0},
	    {"append", 0, 0, 0},
	    {"delete", 1, 0, 0},
	    {"verbose", 0, 0, 'v'},
	    {"config", 1, 0, 'c'},
	    {"repository", 1, 0, 'r'},
	    {"force", 0, 0, 'f'},
	    {0, 0, 0, 0}
	};

	int c = getopt_long(argc, argv, "abc:d:vr:f", long_options, &option_index);
	if (c == -1)
	    break;

	switch (c) {
	case 0:
	    std::cout << "option " << long_options[option_index].name;
	    if (optarg)
		std::cout << " with arg " << optarg;
	    std::cout << std::endl;
	    break;

	case '0':
	case '1':
	case '2':
	    if (digit_optind != 0 && digit_optind != this_option_optind)
	      std::cout << "digits occur in two different argv-elements." << std::endl;
	    digit_optind = this_option_optind;
	    std::cout << "option " << c << std::endl;
	    break;


	case 'a':
	    std::cout << "option a" << std::endl;
	    break;

	case 'v':
            verbose = true;
	    break;

	case 'f':
            force = true;
	    break;

	case 'c':
          // TODO
	    loadConfigFile(optarg);
	    break;

	case 'r':
	  // Chose another repository:
	    repository = optarg;
	    break;

	case 'd':
	    std::cout << "option d with value '" << optarg << "'" << std::endl;
	    break;

	case '?':
	    break;

	default:
	    std::cout << "?? getopt returned character code " << c << "??" << std::endl;
	}
    }

    if (optind < argc) {
	while (optind < argc) {
          if (this->operation.empty()) {
                // First argument is OPERATION:
	        this->operation = argv[optind++];
	    } else {
		std::cout << "non-option ARGV-element: " << argv[optind++] << std::endl;
	    }
	}
    }
}


void RuntimeConfig::load()
{
  File home;

  //this->tryToLoadFrom(SYSCONFDIR "/shaback/conf.d");
  tryToLoadFrom("etc/shaback/conf.d");

  File localConfig(home, ".shaback.lua");
  if (localConfig.isFile()) {
    loadConfigFile(localConfig.path);
  }
}

void RuntimeConfig::tryToLoadFrom(string dir)
{
  vector<File> files = File(dir).listFiles("*.lua");
  // TODO: Pattern

  for (vector<File>::iterator it = files.begin(); it < files.end(); it++ ) {
    File f(*it);
    loadConfigFile(f.path);
  }
}


void RuntimeConfig::loadConfigFile(std::string filename)
{
  int error = luaL_dofile (this->luaState, filename.c_str());
  if (error) {
    std::cerr << lua_tostring(this->luaState, -1) << std::endl;
    lua_pop(this->luaState, 1);  /* pop error message from the stack */
    exit(2);
  }
}


static RuntimeConfig* getRuntimeConfig(lua_State *L, int stackPos)
{
  lua_getglobal(L, LUA_RUNTIMECONFIG);
  return (RuntimeConfig*) lua_touserdata(L, stackPos);
}


static int l_repository (lua_State *L) {
  const char* dir = lua_tostring(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->repository = dir;

  return 0;
}


static int l_oneFileSystem (lua_State *L) {
  bool b = (bool) lua_toboolean(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->oneFileSystem = b;

  return 0;
}


static int l_verbose (lua_State *L) {
  bool b = (bool) lua_toboolean(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->verbose = b;

  return 0;
}


static int l_localCache (lua_State *L) {
  const char* file = lua_tostring(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->localCacheFile = file;

  return 0;
}


static int l_addDir (lua_State *L) {
  const char* dir = lua_tostring(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->dirs.push_back(dir);

  return 0;
}


static int l_addExcludePattern (lua_State *L) {
  const char* pattern = lua_tostring(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->excludePatterns.push_back(pattern);
  
  return 0;
}


static int l_addSplitPattern (lua_State *L) {
  const char* pattern = lua_tostring(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->splitPatterns.push_back(pattern);

  return 0;
}


void RuntimeConfig::initLua()
{
  this->luaState = luaL_newstate();
  luaL_openlibs(this->luaState);

  lua_pushcfunction(this->luaState, l_repository);
  lua_setglobal(this->luaState, "repository");

  lua_pushcfunction(this->luaState, l_localCache);
  lua_setglobal(this->luaState, "localCache");

  lua_pushcfunction(this->luaState, l_oneFileSystem);
  lua_setglobal(this->luaState, "oneFileSystem");

  lua_pushcfunction(this->luaState, l_verbose);
  lua_setglobal(this->luaState, "verbose");

  lua_pushcfunction(this->luaState, l_addDir);
  lua_setglobal(this->luaState, "addDir");

  lua_pushcfunction(this->luaState, l_addExcludePattern);
  lua_setglobal(this->luaState, "addExcludePattern");

  lua_pushcfunction(this->luaState, l_addSplitPattern);
  lua_setglobal(this->luaState, "addSplitPattern");

  lua_pushlightuserdata(this->luaState, this);
  lua_setglobal(this->luaState, LUA_RUNTIMECONFIG);
}

void RuntimeConfig::finalize()
{
  File repo(this->repository);
  this->filesDir = File(repo, "files");
  this->indexDir = File(repo, "index");
  this->locksDir = File(repo, "locks");
  this->cacheDir = File(repo, "cache");
}


bool RuntimeConfig::excludeFile(File& file)
{
  for (vector<string>::iterator it = excludePatterns.begin(); it < excludePatterns.end(); it++ ) {
    string pattern(*it);
#ifdef WIN32

#else
    if (fnmatch(pattern.c_str(), file.path.c_str(), 0) == 0) {
      return true;
    }
#endif
  }
  return false;
}
