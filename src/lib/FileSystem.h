/*
 * shaback - A hash digest based backup tool.
 * Copyright (C) 2021 Florian Wolff (florian@donuz.de)
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

#ifndef SHABACK_FileSystem_H
#define SHABACK_FileSystem_H

#include <string>
#include <vector>
#include <sys/stat.h>
#include <stdio.h>
#include "File.h"

#include "config.h"

class FileSystem
{
  public:
    virtual File file(std::string path) = 0;
    virtual File file(File parent, std::string filename) = 0;
};

#endif // SHABACK_FileSystem_H

