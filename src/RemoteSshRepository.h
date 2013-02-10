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

#ifndef SHABACK_RemoteSshRepository_H
#define SHABACK_RemoteSshRepository_H

#include "Repository.h"
#include "lib/Process.h"

class RemoteSshRepository : public Repository
{
  public:
    RemoteSshRepository(RuntimeConfig& config);
    ~RemoteSshRepository();

    void open();

    void lock(bool exclusive = false);

    void unlock();
    void show();

    File hashValueToFile(std::string hashValue);
    bool contains(std::string& hashValue);
    std::string storeTreeFile(BackupRun* run, std::string& treeFile);
    std::string storeFile(BackupRun* run, File& srcFile);
    void storeRootTreeFile(std::string& rootHashValue);
    void importCacheFile();

    void exportCacheFile();

    std::vector<TreeFileEntry> loadTreeFile(std::string& treeId);
    void exportFile(TreeFileEntry& entry, OutputStream& out);

    void exportFile(std::string& id, OutputStream& out);
    void exportSymlink(TreeFileEntry& entry, File& outFile);

    void openReadCache();

    void removeAllCacheFiles();

    ShabackInputStream createInputStream();

    ShabackOutputStream createOutputStream();

  protected:

    Process* sshProcess;

  private:
    char* readBuffer;

};
#endif // SHABACK_RemoteSshRepository_H
