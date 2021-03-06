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

#include "SplitFileIndexReader.h"

using namespace std;

SplitFileIndexReader::SplitFileIndexReader(Repository& repository, string id)
: file(repository.hashValueToFile(id)), in(repository.createInputStream())
{
  in.open(file);
  reader = new BufferedReader(&in);
}

SplitFileIndexReader::~SplitFileIndexReader()
{
  delete reader;
  in.close();
}

bool SplitFileIndexReader::next(string& hashValue)
{
  if (reader->readLine(hashValue)) {
    return (hashValue.size() >= 20);
  } else {
    return false;
  }
}
