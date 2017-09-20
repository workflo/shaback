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

#ifndef SHABACK_GarbageCollection_H
#define SHABACK_GarbageCollection_H

#include <string>
#include <set>
#include "Repository.h"
#include "lib/File.h"
#include "lib/Exception.h"

class GarbageCollection
{
  public:
    GarbageCollection(RuntimeConfig& config, Repository& Repository);
    ~GarbageCollection();

    void run();
    void showTotals();

  protected:
    void processShabackupFile(File& rootFile);
    void reportError(Exception& ex);
    void removeUnusedFiles();

    /**
     * Adds all blocks of a split file to the list
     * of files to be kept.
     */
    void keepSplitFileBlocks(TreeFileEntry& entry);

    Repository& repository;
    RuntimeConfig& config;
    int numErrors;
    int tmpFilesDeleted;
    int filesDeleted;
};

#endif // SHABACK_GarbageCollection_H
