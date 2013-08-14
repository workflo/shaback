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

#ifndef SHABACK_Repository_H
#define SHABACK_Repository_H

#include <string>
#include <vector>
#include "ShabackConfig.h"
#include "RuntimeConfig.h"
#include "Cache.h"
#include "lib/Date.h"
#include "ShabackOutputStream.h"
#include "ShabackInputStream.h"
#include "TreeFileEntry.h"

class BackupRun;

class Repository
{
  public:
    Repository(RuntimeConfig& config);
    ~Repository();

#if defined(SHABACK_HAS_BACKUP)
    int backup();
#endif

    int restore();

    /**
     * Checks whether all required directories and files can be found.
     * Lodas the repository's config file (repo.properties) to determine
     * compression and encryptions parameters.
     */
    void open();

    /**
     * Tries to acquire a read/write lock on the repository.
     *
     * @param exclusive Whether to acquire an exclusive write lock (for garbage collection etc.).
     */
    void lock(bool exclusive = false);

    /**
     * Releases the acquired lock.
     */
    void unlock();

    /**
     * 'show' command. Exports a file or tree file from the repository.
     */
    void show();

    /**
     * 'gc' command. Performs a garbage collection on the repository.
     */
    void gc();

    File hashValueToFile(std::string hashValue);
    bool contains(std::string& hashValue);
#if defined(SHABACK_HAS_BACKUP)
    std::string storeTreeFile(BackupRun* run, std::string& treeFile);
    std::string storeFile(BackupRun* run, File& srcFile, shaback_filesize_t* totalFileSize);
    void storeRootTreeFile(std::string& rootHashValue);
#endif
    void importCacheFile();

    /**
     * Exports all hash digests from the local cache file into
     * a new cache file in the repository's cache/ directory.
     */
    void exportCacheFile();

    std::vector<TreeFileEntry> loadTreeFile(std::string& treeId);
    void exportFile(TreeFileEntry& entry, OutputStream& out);

    /**
     * Reads the backup file represented by the given ID to the given
     * \c OutputStream.
     */
    void exportFile(std::string& id, OutputStream& out);
    void exportSymlink(TreeFileEntry& entry, File& outFile);

    /**
     * Lazily opens the read cache.
     */
    void openReadCache();

    /**
     * Removes all files from the repository's cache/ directory.
     */
    void removeAllCacheFiles();

    /**
     * Returns a new \c ShabackInputStream configured to read a file from this
     * repository.
     */
    ShabackInputStream createInputStream(bool allowCaching = true);

    /**
     * Returns a new \c ShabackOutputStream configured to write a file to this
     * repository.
     */
    ShabackOutputStream createOutputStream(bool allowCaching = true);

    /**
     * Maps the given name of an encryption algorithm to its respective
     * constant.
     *
     * @throws UnsupportedEncryptionAlgorithm If the given algorithm is not known.
     */
    static int encryptionByName(std::string name);

    /**
     * Maps the given name of a compression algorithm to its respective
     * constant.
     *
     * @throws UnsupportedCompressionAlgorithm If the given algorithm is not known.
     */
    static int compressionByName(std::string name);

    /**
     * Maps the given name of a repository format to its respective
     * constant.
     *
     * @throws UnsupportedRepositoryFormat If the given format is not known.
     */
    static int repoFormatByName(std::string name);

#if defined(SHABACK_HAS_BACKUP)
    /**
     * Returns the hash digest for the given password as a hex string.
     */
    static std::string hashPassword(std::string password);
#endif

    /**
     * Maps the internal compression algorithm's number to a human-readable name.
     */
    static std::string compressionToName(int compression);

    /**
     * Maps the internal encryption algorithm's number to a human-readable name.
     */
    static std::string encryptionToName(int encryption);

    /**
     * Maps the internal repository format's number to a human-readable name.
     */
    static std::string repoFormatToName(int fmt);

    /** The temporary write cache. Used to speed up backup. */
    std::set<std::string> writeCache;

    /** The (persistent) read cache. Used to speed up traversing tree files. */
    Cache readCache;

    int repoFormat;

  protected:
    RuntimeConfig config;
    int hashAlgorithm;
    int encryptionAlgorithm;
    int compressionAlgorithm;
    int splitBlockSize;  // Must be a multiple auf 512 to allow O_DIRECT file IO mode!
    int splitMinBlocks;
    Date startDate;

    int restoreByRootFile(File& rootFile);
    int restoreByTreeId(std::string& treeId);

#if defined(SHABACK_HAS_BACKUP)
    void checkPassword();
#endif

  private:
    /**
     * Splits the input stream into chunks and stores them
     * individually.
     */
    std::string storeSplitFile(BackupRun* run, File &srcFile, InputStream &in, shaback_filesize_t* totalFileSize);

    void* readBuffer;
};

#define COMPRESSION_NONE    0
#define COMPRESSION_DEFLATE 1
#define COMPRESSION_BZip5   10
#define COMPRESSION_BZip1   11
#define COMPRESSION_BZip9   12
#define COMPRESSION_LZMA0   20
#define COMPRESSION_LZMA5   21
#define COMPRESSION_LZMA9   22

#define ENCRYPTION_NONE     0
#define ENCRYPTION_BLOWFISH 1
//#define ENCRYPTION_TWOFISH  2
//#define ENCRYPTION_AES      3
//#define ENCRYPTION_DES      4

#define DIGEST_SHA1         1
#define DIGEST_SHA256       2

#define REPOFORMAT_2_2      0
#define REPOFORMAT_3        1

// Don't touch me!
#define PASSWORDFILE_SALT "This salt makes the hashed password useless for decryption"

#define SPLITFILE_ID_INDICATOR_STR "_s"
#define SPLITFILE_ID_INDICATOR 's'

#endif // SHABACK_Repository_H
