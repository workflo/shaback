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
#include "RuntimeConfig.h"
#include "Cache.h"
#include "lib/Date.h"
#include "TreeFileEntry.h"

class BackupRun;

class Repository
{
  public:
    Repository(RuntimeConfig& config);
    ~Repository();

    int backup();
    void restore();

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
    std::string storeTreeFile(BackupRun* run, std::string& treeFile);
    std::string storeFile(BackupRun* run, File& srcFile);
    void storeRootTreeFile(std::string& rootHashValue);
    void importCacheFile();
    void exportCacheFile();
    std::vector<TreeFileEntry> loadTreeFile(std::string& treeId);
    void exportFile(std::string& id, OutputStream& out);
    void exportSymlink(TreeFileEntry& entry, File& outFile);
    void openCache();

    Cache cache;

  protected:
    RuntimeConfig config;
    int hashAlgorithm;
    int encryptionAlgorithm;
    int compressionAlgorithm;
    Date startDate;

    void restoreByRootFile(File& rootFile);
    void restoreByTreeId(std::string& treeId);
};

#define COMPRESSION_NONE    0
#define COMPRESSION_DEFLATE 1

#define ENCRYPTION_NONE     0
#define ENCRYPTION_BLOWFISH 1
#define ENCRYPTION_AES      2
#define ENCRYPTION_DES      3

#define DIGEST_SHA1         1
#define DIGEST_SHA256       2

#endif // SHABACK_Repository_H
