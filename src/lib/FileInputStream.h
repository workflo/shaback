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

#ifndef SHABACK_FileInputStream_H
#define SHABACK_FileInputStream_H

#include <string.h>
#ifdef WIN32
# include <windows.h>
#endif
#include "InputStream.h"
#include "File.h"

/**
 * This class is a stream that reads its bytes from a file.
 *
 * @class FileInputStream
 */
class FileInputStream: public InputStream
{
  public:

    /**
     * Opens the specified file for reading.
     */
    FileInputStream(File& file);

    /**
     * Opens the specified file for reading.
     */
    FileInputStream(const char* filename);

    /**
     * Opens the specified file for reading.
     */
    FileInputStream(std::string& filename);

    ~FileInputStream();

    int read();

    int read(char* b, int len);

    void close();

    void reset();

  protected:
#ifdef WIN32
    HANDLE handle;
#else
    int handle;
#endif

    void init(std::string& filename);

};
#endif // SHABACK_FileInputStream_H
