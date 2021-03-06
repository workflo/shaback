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

#ifndef SHABACK_SplitFileIndexReader_H
#define SHABACK_SplitFileIndexReader_H

#include <string>

#include "lib/File.h"
#include "lib/BufferedReader.h"

#include "Repository.h"
#include "ShabackInputStream.h"

class SplitFileIndexReader
{
  public:
    SplitFileIndexReader(Repository& repository, std::string id);
    virtual ~SplitFileIndexReader();

    /**
     * Tries to read the next hash value from the index file.
     *
     * @param hashValue Output parameter, will contains the next hash value if available.
     * @return Whether a hash value could be loaded.
     */
    virtual bool next(std::string& hashValue);

  protected:
    File file;
    ShabackInputStream in;
    BufferedReader* reader;
};

#endif /* SHABACK_SplitFileIndexReader_H */
