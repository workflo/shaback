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
#include <set>
#include "lib/File.h"

extern "C" {
# include <lua.h>
}
class BackupRun;

class RuntimeConfig
{
  public:
    RuntimeConfig();
    ~RuntimeConfig();
    virtual void load();
    virtual void parseCommandlineArgs(int argc, char** argv);
    virtual void loadConfigFile(std::string filename);
#if defined(OPENSSL_FOUND)
    unsigned char* derivedKey();
#endif

    bool quiet;
    int verbose;
    bool debug;
    bool gauge;
    bool force;
    bool oneFileSystem;
    bool showTotals;
    bool help;
    bool useWriteCache;
    bool useReadCache;
    bool useSymlinkLock;
    bool skipExisting;
    bool restoreAsCpioStream;
    bool restoreAsShabackStream;
    bool gui;
    bool all;
    bool quick;
    bool actionList;
    bool actionDetails;
    int backupsToKeep;
    int number;

    std::string color_error;
    std::string color_success;
    std::string color_filename;
    std::string color_stats;
    std::string color_deleted;
    std::string color_default;
    std::string color_low;
    std::string color_debug;

    std::string style_bold;
    std::string style_default;

    std::string operation;
    std::string repository;

    /** The temporary write cache file. */
    File writeCacheFile;

    /** The persistent read cache file. */
    File readCacheFile;

    std::string backupName;
    lua_State* luaState;
    std::vector<std::string> excludePatterns;

    /** Filename/path patterns of files to be split into blocks. */
    std::vector<std::string> splitPatterns;

    /** Split only files that are at least this size. */
    long long splitFileMinSize;

    /** Block size for splitting large files. */
    int splitFileBlockSize;

    /** List of directories to be backed up. */
    std::vector<std::string> dirs;
    std::vector<std::string> cliArgs;

    /** Clear-text password when using an encryted repository. */
    std::string cryptoPassword;

    /** The crypto key derived from cryptoPassword using PKCS5_PBKDF2_HMAC_SHA1. Always EVP_MAX_KEY_LENGTH (=64) bytes. */
    unsigned char* cryptoKey;

    /** Encryption algorithm to be used when creating a new repository. */
    int init_encryptionAlgorithm;

    /** Compression algorithm to be used when creating a new repository. */
    int init_compressionAlgorithm;

    /** Repository format */
    int init_repoFormat;

    File filesDir;
    File indexDir;
    File locksDir;
    File cacheDir;
    File repoDir;
    bool haveExclusiveLock;
    int lockCount;
    
    /** Set of errors to be ignored. */
    std::set<std::string> ignoreErrors;

    /**
     * Out non-exclusive lock file.
     */
    File lockFile;
    
    /**
     * The globally exclusive lock file.
     */
    File exclusiveLockFile;

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

    /**
     * Boundaries for keeping old backups:
     * - Keep all backups for n[0] days,
     * - keep daily backup for n[1] days,
     * - keep weekly backup for n[2] days
     * - and keep monthly backup for the remainder.
     */
    int keepOldBackupsBoundaries[3];

    /**
     * Determines whether this file should be excluded from the backup set.
     */
    bool excludeFile(File& file);

    /**
     * Determines whether this file should be split into blocks
     * according to its name and size.
     */
    bool splitFile(File& file);

    void finalize();

    /**
     * Calls all pre-backup callbacks.
     */
    void runPreBackupCallbacks();

    /**
     * Calls all post-backup callbacks.
     */
    void runPostBackupCallbacks(BackupRun *run);

    /**
     * Calls all enter-directory callbacks.
     */
    void runEnterDirCallbacks(File &dir);

    /**
     * Calls all leave-directory callbacks.
     */
    void runLeaveDirCallbacks(File &dir);

  protected:
    void initLua();
    void tryToLoadFrom(std::string dir);
};

#endif // SHABACK_RuntimeConfig_H
