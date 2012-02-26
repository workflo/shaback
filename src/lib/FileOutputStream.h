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

#ifndef SHABACK_FileOutputStream_H
#define SHABACK_FileOutputStream_H

#include <string.h>
#ifdef WIN32
# include <windows.h>
#endif
#include "File.h"
#include "OutputStream.h"

/**
 * This classes allows a stream of data to be written to a disk file.
 *
 * @class FileOutputStream
 */
class FileOutputStream: public OutputStream
{
  public:
    FileOutputStream(File& file);
    FileOutputStream(const char* filename);
    FileOutputStream(std::string& filename);
    ~FileOutputStream();

    void write(int b);
    void write(const char* b, int len);
    void close();

  protected:
#ifdef WIN32
    HANDLE handle;
#else
    FILE* handle;
#endif
    void init(std::string& filename);
};
#endif // SHABACK_FileOutputStream_H
