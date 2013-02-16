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

#ifndef SHABACK_BackupRun_H
#define SHABACK_BackupRun_H

#include <string>
#include "ShabackConfig.h"
#include "Repository.h"
#include "lib/File.h"
#include "lib/Exception.h"

class BackupRun
{
  public:
    BackupRun(RuntimeConfig& config, Repository& Repository);
    ~BackupRun();

    int run();
    void showTotals();
    void reportError(Exception& ex);

    shaback_filesize_t numBytesRead;
    shaback_filesize_t numBytesStored;
    int numFilesRead;
    int numFilesStored;
    int numErrors;

  protected:
    std::string handleDirectory(File& dir, bool absolutePaths, bool skipChildren = false);
    std::string handleFile(File& dir, bool absolutePaths);
    std::string handleSymlink(File& dir, bool absolutePaths);
    void handleSymlink(File& dir);

    /**
     * Removes old index files. Keeps all index files from within the past week,
     * one file per week for the last month and one file per month for older files.
     */
    void deleteOldIndexFiles();

    Repository& repository;
    RuntimeConfig& config;
};

#endif // SHABACK_BackupRun_H
