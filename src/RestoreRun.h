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

#ifndef SHABACK_RestoreRun_H
#define SHABACK_RestoreRun_H

#include <string>
#include <stdint.h>
#include <time.h>
#include "Repository.h"
#include "RestoreReport.h"

class RestoreRun
{
  public:
    RestoreRun(RuntimeConfig& config, Repository& Repository, File shabackupFile, bool testRestore);
    ~RestoreRun();

    void restore(std::string& treeId, File& destinationDir, int depth = 0);
    void restoreAsCpioStream(std::string& treeId, int depth = 0);

    RestoreReport start(std::list<std::string> files, File& destinationDir);
    RestoreReport report;

  protected:
    void restoreMetaData(File& file, TreeFileEntry& entry);
    void reportError(std::string msg);

  private:
    Repository& repository;
    RuntimeConfig& config;
    void progress(std::string &path);
    time_t lastProgressTime;
    bool testRestore;
    File shabackupFile;
};

#endif // SHABACK_RestoreRun_H
