/*
 * shaback - A hash digest based backup tool.
 * Copyright (C) 2012 Florian Wolff (florian@donuz.de)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
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
#include "ShabackConfig.h"

#define LUA_RUNTIMECONFIG "__RuntimeConfig__"

using namespace std;

RuntimeConfig::RuntimeConfig()
{
  repository = ".";
  verbose = false;
  debug = false;
  oneFileSystem = false;
  showTotals = false;
  help = false;
  force = false;
  backupName = "noname";
  initLua();
}

void RuntimeConfig::parseCommandlineArgs(int argc, char** argv)
{
  while (true) {
    int option_index = 0;
    static struct option long_options[] = { { "debug", no_argument, 0, 'd' }, { "verbose", no_argument, 0, 'v' }, {
        "totals", no_argument, 0, 't' }, { "config", required_argument, 0, 'c' }, { "repository", required_argument, 0,
        'r' }, { "force", no_argument, 0, 'f' }, { "password", required_argument, 0, 'p' }, { "name",
        required_argument, 0, 'n' }, { "help", no_argument, 0, 'h' }, { 0, 0, 0, 0 } };

    int c = getopt_long(argc, argv, "c:dvtr:fp:n:h", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
      case 'v':
        verbose = true;
        break;

      case 't':
        showTotals = true;
        break;

      case 'f':
        force = true;
        break;

      case 'h':
        help = true;
        break;

      case 'c':
        loadConfigFile(optarg);
        break;

      case 'r':
        repository = optarg;
        break;

      case 'n':
        backupName = optarg;
        break;

      case 'd':
        debug = true;
        break;

      case 'p':
        cryptoPassword = optarg;
        break;

      default:
        cerr << "?? getopt returned character code " << c << "??" << std::endl;
    }
  }

  if (optind < argc) {
    while (optind < argc) {
      if (operation.empty()) {
        // First argument is OPERATION:
        operation = argv[optind++];
      } else {
        cliArgs.push_back(argv[optind++]);
      }
    }
  }
}

void RuntimeConfig::load()
{
  File home;

  tryToLoadFrom(SHABACK_SYSCONFDIR "/shaback/conf.d");

  File localConfig(home, ".shaback.lua");
  if (localConfig.isFile()) {
    loadConfigFile(localConfig.path);
  }
}

void RuntimeConfig::tryToLoadFrom(string dir)
{
  vector<File> files = File(dir).listFiles("*.lua");

  for (vector<File>::iterator it = files.begin(); it < files.end(); it++) {
    File f(*it);
    loadConfigFile(f.path);
  }
}

void RuntimeConfig::loadConfigFile(std::string filename)
{
  int error = luaL_dofile (this->luaState, filename.c_str());
  if (error) {
    std::cerr << lua_tostring(this->luaState, -1) << std::endl;
    lua_pop(this->luaState, 1); /* pop error message from the stack */
    exit(2);
  }
}

static RuntimeConfig* getRuntimeConfig(lua_State *L, int stackPos)
{
  lua_getglobal(L, LUA_RUNTIMECONFIG);
  return (RuntimeConfig*) lua_touserdata(L, stackPos);
}

static int l_repository(lua_State *L)
{
  const char* dir = lua_tostring(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->repository = dir;

  return 0;
}

static int l_oneFileSystem(lua_State *L)
{
  bool b = (bool) lua_toboolean(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->oneFileSystem = b;

  return 0;
}

static int l_verbose(lua_State *L)
{
  bool b = (bool) lua_toboolean(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->verbose = b;

  return 0;
}

static int l_showTotals(lua_State *L)
{
  bool b = (bool) lua_toboolean(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->showTotals = b;

  return 0;
}

static int l_localCache(lua_State *L)
{
  const char* file = lua_tostring(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->localCacheFile = file;

  return 0;
}

static int l_addDir(lua_State *L)
{
  const char* dir = lua_tostring(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->dirs.push_back(dir);

  return 0;
}

static int l_clearDirs(lua_State *L)
{
  RuntimeConfig* config = getRuntimeConfig(L, 1);
  config->dirs.clear();

  return 0;
}

static int l_addExcludePattern(lua_State *L)
{
  const char* pattern = lua_tostring(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->excludePatterns.push_back(pattern);

  return 0;
}

static int l_addSplitPattern(lua_State *L)
{
  const char* pattern = lua_tostring(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->splitPatterns.push_back(pattern);

  return 0;
}

static int l_cryptoPassword(lua_State *L)
{
  const char* pw = lua_tostring(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->cryptoPassword = pw;

  return 0;
}

static int l_backupName(lua_State *L)
{
  const char* n = lua_tostring(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->backupName = n;

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

  lua_pushcfunction(this->luaState, l_showTotals);
  lua_setglobal(this->luaState, "showTotals");

  lua_pushcfunction(this->luaState, l_addDir);
  lua_setglobal(this->luaState, "addDir");

  lua_pushcfunction(this->luaState, l_clearDirs);
  lua_setglobal(this->luaState, "clearDirs");

  lua_pushcfunction(this->luaState, l_addExcludePattern);
  lua_setglobal(this->luaState, "addExcludePattern");

  lua_pushcfunction(this->luaState, l_addSplitPattern);
  lua_setglobal(this->luaState, "addSplitPattern");

  lua_pushcfunction(this->luaState, l_cryptoPassword);
  lua_setglobal(this->luaState, "cryptoPassword");

  lua_pushcfunction(this->luaState, l_backupName);
  lua_setglobal(this->luaState, "backupName");

  lua_pushlightuserdata(this->luaState, this);
  lua_setglobal(this->luaState, LUA_RUNTIMECONFIG);
}

void RuntimeConfig::finalize()
{
  repoDir = File(repository);
  filesDir = File(repoDir, "files");
  indexDir = File(repoDir, "index");
  locksDir = File(repoDir, "locks");
  cacheDir = File(repoDir, "cache");
  repoPropertiesFile = File(repoDir, "repo.properties");
}

bool RuntimeConfig::excludeFile(File& file)
{
  for (vector<string>::iterator it = excludePatterns.begin(); it < excludePatterns.end(); it++) {
    string pattern(*it);
#ifdef WIN32
    // TODO: WIN32: fnmatch
#else
    if (fnmatch(pattern.c_str(), file.path.c_str(), 0) == 0) {
      return true;
    }
#endif
  }
  return false;
}
