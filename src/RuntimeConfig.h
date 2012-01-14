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

    /** List of directories to be backed up. */
    std::vector<std::string> dirs;
    std::vector<std::string> cliArgs;

    /** Clear-text password when using an encryted repository. */
    std::string cryptoPassword;

    /** Encryption algorithm to be used when creating a new repository. */
    int init_encryptionAlgorithm;

    /** Compression algorithm to be used when creating a new repository. */
    int init_compressionAlgorithm;

    File filesDir;
    File indexDir;
    File locksDir;
    File cacheDir;
    File repoDir;

    /**
     * This (.properties) file contains basic settings like the selected
     * compression and encrytion algorithms.
     */
    File repoPropertiesFile;

    /**
     * If encryption is enabled, this file contains the encrypted
     * (and optionally deflated) SHA-256 digest of the chosen password.
     * This way shaback can check whether the given password is correct.
     */
    File passwordCheckFile;

    bool excludeFile(File& file);

    void finalize();

  protected:
    void initLua();
    void tryToLoadFrom(std::string dir);
};

#endif // SHABACK_RuntimeConfig_H
