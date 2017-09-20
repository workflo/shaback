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
# include <termios.h>
#endif
#include <stdlib.h>
#include <string.h>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "RuntimeConfig.h"
#include "ShabackConfig.h"
#include "Repository.h"
#include "BackupRun.h"

#define LUA_RUNTIMECONFIG "__RuntimeConfig__"

using namespace std;

RuntimeConfig::RuntimeConfig()
{
  repository = ".";
  lockCount = 0;
  quiet = false;
  verbose = 0;
  debug = false;
  oneFileSystem = false;
  showTotals = false;
  help = false;
  force = false;
  haveExclusiveLock = false;
  useWriteCache = true;
  useSymlinkLock = true;
  skipExisting = false;
  restoreAsCpioStream = false;
  restoreAsShabackStream = false;
  gauge = false;
  gui = false;
  all = false;
  quick = false;
  actionList = false;
  actionDetails = false;
  backupsToKeep = -1;
  number = 0;
  cryptoKey = 0;
  init_compressionAlgorithm = COMPRESSION_DEFLATE;
  init_encryptionAlgorithm = ENCRYPTION_NONE;
  init_repoFormat = REPOFORMAT_2_2;

  backupName = "noname";
  splitFileBlockSize = 1024 * 1024 * 5;
  splitFileMinSize = splitFileBlockSize * 5;
  keepOldBackupsBoundaries[0] = 1;    // Keep all backups for n days
  keepOldBackupsBoundaries[1] = 14;   // Keep daily backup for n days
  keepOldBackupsBoundaries[2] = 30;   // Keep weekly backup for n days

  // Temporary write cache:
  char cacheFileName[40];
  sprintf(cacheFileName, "shaback-write-cache-%d.gdbm", getpid());
  writeCacheFile = File(File::tmpdir(), cacheFileName);

  // Read cache:
  readCacheFile = File(File::home(), ".shaback-read-cache.gdbm");

  initLua();

  struct termios terminal;
  if (tcgetattr(0, &terminal) != -1 && tcgetattr(1, &terminal) != -1) {
    // Colors (See http://misc.flogisoft.com/bash/tip_colors_and_formatting)
    color_error = "\e[31m";
    color_success = "\e[32m";
    color_filename = "\e[36m";
    color_stats = "\e[34m";
    color_deleted = "\e[35m";
    color_default = "\e[39m";

    style_bold = "\e[1m";
    style_default = "\e[0m";
  }
}

RuntimeConfig::~RuntimeConfig()
{
  writeCacheFile.remove();
}

void RuntimeConfig::parseCommandlineArgs(int argc, char** argv)
{
  while (true) {
    int option_index = 0;
    static struct option long_options[] = { { "debug", no_argument, 0, 'd' }, { "verbose", no_argument, 0, 'v' }, {
        "totals", no_argument, 0, 't' }, { "config", required_argument, 0, 'c' }, { "repository", required_argument, 0,
        'r' }, { "force", no_argument, 0, 'f' }, { "password", required_argument, 0, 'p' }, { "name", required_argument,
        0, 'n' }, { "help", no_argument, 0, 'h' }, { "encryption", required_argument, 0, 'E' }, { "compression",
        required_argument, 0, 'C' }, {"ignore-error", required_argument, 0, 'i'},
        {"repo-format", required_argument, 0, 'F'},
        {"cpio", no_argument, 0, 'o'},
        {"shaback", no_argument, 0, 'O'},
        {"no-write-cache", no_argument, 0, 'W'},
        {"no-symlink-lock", no_argument, 0, 'L'},
        {"skip-existing", no_argument, 0, 'S'},
        {"quiet", no_argument, 0, 'q'},
        {"gauge", no_argument, 0, 'G'},
        {"all", no_argument, 0, 'a'},
        {"quick", no_argument, 0, 'Q'},
        {"list", no_argument, 0, 'l'},
        {"details", no_argument, 0, 'D'},
        {"keep", required_argument, 0, 'k'},
        {"1", no_argument, 0, '1'},
#if defined(HAVE_DIALOG)        
        {"gui", no_argument, 0, 'g'},
#endif
        { 0, 0, 0, 0 } };

    int c = getopt_long(argc, argv, "c:dvtr:fp:n:hE:C:F:i:WLSoOqGgaQlk:D1", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
      case 'v':
        verbose++;
        break;

      case 't':
        showTotals = true;
        break;

      case 'f':
        force = true;
        break;

      case 'q':
        quiet = true;
        gauge = true;
        break;

      case 'h':
        help = true;
        break;

      case 'c':
        loadConfigFile(optarg);
        break;

      case 'i':
        ignoreErrors.insert(optarg);
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

      case 'o':
        restoreAsCpioStream = true;
        break;

      case 'G':
        gauge = true;
        break;

      case 'a':
        all = true;
        break;

      case 'Q':
        quick = true;
        break;

#if defined(HAVE_DIALOG)
      case 'g':
        gui = true;
        break;
#endif

      case 'O':
        restoreAsShabackStream = true;
        break;

      case 'p':
        cryptoPassword = optarg;
        break;

      case 'E':
        init_encryptionAlgorithm = Repository::encryptionByName(optarg);
        break;

      case 'C':
        init_compressionAlgorithm = Repository::compressionByName(optarg);
        break;

      case 'F':
        init_repoFormat = Repository::repoFormatByName(optarg);
        break;

      case 'W':
        useWriteCache = false;
        break;
        
      case 'L':
        useSymlinkLock = false;
        break;

      case 'S':
        skipExisting = true;
        break;

      case 'l':
        actionList = true;
        break;

      case 'D':
        actionDetails = true;
        break;

      case 'k':
        backupsToKeep = atoi(optarg);
        break;

      case '1':
        number = 1;
        break;

      default:
        char cBuf[2];
        cBuf[0] = c; cBuf[1] = 0;
        cerr << "Invalid command line option `" << cBuf << "'" << std::endl;
        exit(1);
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

  struct termios terminal;
  if (tcgetattr(0, &terminal) != -1 && tcgetattr(1, &terminal) != -1) {
    // Colors (See http://misc.flogisoft.com/bash/tip_colors_and_formatting)
    color_error = "\e[31m";
    color_success = "\e[32m";
    color_filename = "\e[36m";
    color_stats = "\e[34m";
    color_default = "\e[39m";
    color_low = "\e[90m";
    color_debug = "\e[37m";

    style_bold = "\e[1m";
    style_default = "\e[0m";
  }
}

void RuntimeConfig::load()
{
  File home = File::home();

  tryToLoadFrom(SHABACK_SYSCONFDIR "/shaback/conf.d");

  File localConfig(home, ".shaback.lua");
  if (localConfig.isFile()) {
    loadConfigFile(localConfig.path);
  }

  File repoConfig(repository, "repo.lua");
  if (repoConfig.isFile()) {
    loadConfigFile(repoConfig.path);
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
    lua_pop(this->luaState, 1);
    /* pop error message from the stack */
    exit(2);
  }
}

static RuntimeConfig* getRuntimeConfig(lua_State *L, int stackPos)
{
  lua_getglobal(L, LUA_RUNTIMECONFIG);
  return (RuntimeConfig*) lua_touserdata(L, stackPos);
}

static int l_setRepository(lua_State *L)
{
  const char* dir = lua_tostring(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->repository = dir;

  return 0;
}

static int l_setOneFileSystem(lua_State *L)
{
  bool b = (bool) lua_toboolean(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->oneFileSystem = b;

  return 0;
}

static int l_setVerbose(lua_State *L)
{
  bool b = (bool) lua_toboolean(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->verbose = b;

  return 0;
}

static int l_setShowTotals(lua_State *L)
{
  bool b = (bool) lua_toboolean(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->showTotals = b;

  return 0;
}

static int l_setUseSymlinkLock(lua_State *L)
{
  bool b = (bool) lua_toboolean(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->useSymlinkLock = b;

  return 0;
}

static int l_localCache(lua_State *L)
{
  const char* file = lua_tostring(L, 1);

  cerr << "Lua function localCache() is deprecated. Env vars SHABACK_TMP, TMPDIR, TMP, TEMP are used instead." << endl;

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

static int l_setCryptoPassword(lua_State *L)
{
  const char* pw = lua_tostring(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->cryptoPassword = pw;

  return 0;
}

static int l_setBackupName(lua_State *L)
{
  const char* n = lua_tostring(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->backupName = n;

  return 0;
}

static int l_ignoreError(lua_State *L)
{
  const char* error = lua_tostring(L, 1);

  RuntimeConfig* config = getRuntimeConfig(L, 2);
  config->ignoreErrors.insert(error);

  return 0;
}

static int l_setKeepOldBackupsBoundaries(lua_State *L)
{
  RuntimeConfig* config = getRuntimeConfig(L, 4);
  config->keepOldBackupsBoundaries[0] = lua_tointeger(L, 1);
  config->keepOldBackupsBoundaries[1] = lua_tointeger(L, 2);
  config->keepOldBackupsBoundaries[2] = lua_tointeger(L, 3);

  return 0;

}

void RuntimeConfig::initLua()
{
  this->luaState = luaL_newstate();
  luaL_openlibs(this->luaState);

  lua_pushcfunction(this->luaState, l_setRepository);
  lua_setglobal(this->luaState, "repository");
  lua_pushcfunction(this->luaState, l_setRepository);
  lua_setglobal(this->luaState, "setRepository");

  lua_pushcfunction(this->luaState, l_localCache);
  lua_setglobal(this->luaState, "localCache");

  lua_pushcfunction(this->luaState, l_setOneFileSystem);
  lua_setglobal(this->luaState, "oneFileSystem");
  lua_pushcfunction(this->luaState, l_setOneFileSystem);
  lua_setglobal(this->luaState, "setOneFileSystem");

  lua_pushcfunction(this->luaState, l_setVerbose);
  lua_setglobal(this->luaState, "verbose");
  lua_pushcfunction(this->luaState, l_setVerbose);
  lua_setglobal(this->luaState, "setVerbose");

  lua_pushcfunction(this->luaState, l_setShowTotals);
  lua_setglobal(this->luaState, "showTotals");
  lua_pushcfunction(this->luaState, l_setShowTotals);
  lua_setglobal(this->luaState, "setShowTotals");

  lua_pushcfunction(this->luaState, l_setUseSymlinkLock);
  lua_setglobal(this->luaState, "setUseSymlinkLock");

  lua_pushcfunction(this->luaState, l_addDir);
  lua_setglobal(this->luaState, "addDir");

  lua_pushcfunction(this->luaState, l_clearDirs);
  lua_setglobal(this->luaState, "clearDirs");

  lua_pushcfunction(this->luaState, l_addExcludePattern);
  lua_setglobal(this->luaState, "addExcludePattern");

  lua_pushcfunction(this->luaState, l_addSplitPattern);
  lua_setglobal(this->luaState, "addSplitPattern");

  lua_pushcfunction(this->luaState, l_setCryptoPassword);
  lua_setglobal(this->luaState, "cryptoPassword");
  lua_pushcfunction(this->luaState, l_setCryptoPassword);
  lua_setglobal(this->luaState, "setCryptoPassword");

  lua_pushcfunction(this->luaState, l_setBackupName);
  lua_setglobal(this->luaState, "backupName");
  lua_pushcfunction(this->luaState, l_setBackupName);
  lua_setglobal(this->luaState, "setBackupName");

  lua_pushcfunction(this->luaState, l_setKeepOldBackupsBoundaries);
  lua_setglobal(this->luaState, "setKeepOldBackupsBoundaries");

  lua_pushlightuserdata(this->luaState, this);
  lua_setglobal(this->luaState, LUA_RUNTIMECONFIG);
}

void RuntimeConfig::finalize()
{
  char pid[20];
  sprintf(pid, "%u", getpid());

  repoDir = File(repository);
  filesDir = File(repoDir, "files");
  indexDir = File(repoDir, "index");
  locksDir = File(repoDir, "locks");
  cacheDir = File(repoDir, "cache");
  repoPropertiesFile = File(repoDir, "repo.properties");
  passwordCheckFile = File(repoDir, "password");
  lockFile = File(locksDir, string(backupName).append("-").append(pid).append(".lock"));
  exclusiveLockFile = File(locksDir, "lock");
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

bool RuntimeConfig::splitFile(File& file)
{
  if (file.getSize() >= splitFileMinSize) {
    for (vector<string>::iterator it = splitPatterns.begin(); it < splitPatterns.end(); it++) {
      string pattern(*it);
#ifdef WIN32
      // TODO: WIN32: fnmatch
#else
      if (fnmatch(pattern.c_str(), file.path.c_str(), 0) == 0) {
        return true;
      }
#endif
    }
  }

  return false;
}


void RuntimeConfig::runPreBackupCallbacks()
{
  lua_getglobal(this->luaState, "_runPreBackupCallbacks");
  lua_call(this->luaState, 0, 0);
}


void RuntimeConfig::runPostBackupCallbacks(BackupRun *run)
{
  lua_getglobal(this->luaState, "_runPostBackupCallbacks");
  lua_pushinteger(this->luaState, run->numFilesRead);
  lua_pushinteger(this->luaState, run->numBytesRead);
  lua_pushinteger(this->luaState, run->numFilesStored);
  lua_pushinteger(this->luaState, run->numBytesStored);
  lua_pushinteger(this->luaState, run->numErrors);
  lua_call(this->luaState, 5, 0);
}


void RuntimeConfig::runEnterDirCallbacks(File &dir)
{
  lua_getglobal(this->luaState, "_runEnterDirCallbacks");
  lua_pushstring(this->luaState, dir.path.c_str());
  lua_call(this->luaState, 1, 0);
}


void RuntimeConfig::runLeaveDirCallbacks(File &dir)
{
  lua_getglobal(this->luaState, "_runLeaveDirCallbacks");
  lua_pushstring(this->luaState, dir.path.c_str());
  lua_call(this->luaState, 1, 0);
}


#if defined(OPENSSL_FOUND)
  #include "lib/KeyDerivation.h"

unsigned char* RuntimeConfig::derivedKey()
{
  if (cryptoKey == 0) {
    cryptoKey = KeyDerivation::deriveFromPassword(cryptoPassword);
  }

  return cryptoKey;
}
#endif
