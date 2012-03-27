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

#ifndef SHABACK_Cache_H
#define SHABACK_Cache_H

#include <string>
#include "lib/File.h"
#include "lib/OutputStream.h"
#include "lib/InputStream.h"

extern "C" {
# include <gdbm.h>
}

class Cache
{
  public:
    Cache(File file);
    ~Cache();

    void open(int openMode = GDBM_WRCREAT);

    /**
     * Closes the cache if necessary. Does nothing otherwise.
     */
    void close();
    bool contains(std::string& key);
    void put(std::string& key, std::string& value);
    void put(std::string& key);

    /**
     * Returns the value if existing, the empty string otherwise.
     */
    std::string get(std::string& key);

    void remove(std::string& key);

  private:
    File file;
    GDBM_FILE gdbmFile;
    bool opened;
};

#endif // SHABACK_Cache_H
