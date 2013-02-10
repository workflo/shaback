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

#ifndef SHABACK_LocalRepository_H
#define SHABACK_LocalRepository_H

#include "Repository.h"

class LocalRepository : public Repository
{
  public:
    LocalRepository(RuntimeConfig& config);
    ~LocalRepository();

    int backup();
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
    std::string storeTreeFile(BackupRun* run, std::string& treeFile);
//    std::string storeFile(BackupRun* run, File& srcFile);

    void store(BackupRun* run, File& srcFile, InputStream& in, std::string& hashValue);

    void storeRootTreeFile(std::string& rootHashValue);
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
    ShabackInputStream createInputStream();

    /**
     * Returns a new \c ShabackOutputStream configured to write a file to this
     * repository.
     */
    ShabackOutputStream createOutputStream();

    int remoteCommandListener();

    /** The temporary write cache. Used to speed up backup. */
    std::set<std::string> writeCache;

    /** The (persistent) read cache. Used to speed up traversing tree files. */
    Cache readCache;

    void deleteOldIndexFiles();

  protected:
//    int splitBlockSize;
//    int splitMinBlocks;

    int restoreByRootFile(File& rootFile);
    int restoreByTreeId(std::string& treeId);
    void checkPassword();

  private:
    /**
     * Splits the input stream into chunks and stores them
     * individually.
     */
    void storeSplitFile(BackupRun* run, std::string& hashValue, InputStream &in, ShabackOutputStream &blockFileOut);

//    char* readBuffer;
};

#endif // SHABACK_LocalRepository_H
