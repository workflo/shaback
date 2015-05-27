/*
 * shaback - A hash digest based backup tool.
 * Copyright (C) 2015 Florian Wolff (florian@donuz.de)
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

#ifndef SHABACK_BackupsetSelector_H
#define SHABACK_BackupsetSelector_H

#include <string>
#include <vector>
#include "ShabackConfig.h"

#if defined(HAVE_DIALOG)

#include <ncurses.h>

#include "lib/File.h"
#include "lib/Exception.h"


class Repository;
class RuntimeConfig;

class BackupsetSelector
{
  public:
    BackupsetSelector(Repository& repository, RuntimeConfig& config);
    ~BackupsetSelector();

    std::string start();

  private:
    Repository& repository;
    RuntimeConfig& config;
    std::string setName;
    File rootFile;
    std::string directoryId;

    char recoverLabel[100];
    char backTitle[100];
    
    bool selectSet();
    bool selectVersion();
    bool selectDirectory();
};

#endif // HAVE_DIALOG
#endif // SHABACK_BackupsetSelector_H
