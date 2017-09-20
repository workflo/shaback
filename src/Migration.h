/*
 * shaback - A hash digest based backup tool.
 * Copyright (C) 2017 Florian Wolff (florian@donuz.de)
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

#ifndef SHABACK_Migration_H
#define SHABACK_Migration_H
  
#include <string>
#include <vector>
#include <set>
#include "lib/File.h"
#include "Repository.h"
  
class Migration
{
  public:
    Migration(RuntimeConfig& config, Repository& Repository);

    void run();  
 
  protected:
    void migrate2to3();
    void migrate2to3int(ShabackOutputStream& out, std::string& treeId);
    std::vector<File> listRootFiles();

    Repository& repository;
    RuntimeConfig& config;
};
  
#endif // SHABACK_Migration_H
