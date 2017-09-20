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

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
//  #include <string.h>
 
#include "lib/Exception.h"
#include "lib/FileInputStream.h"
#include "lib/BufferedReader.h"
 
#include "DirectoryFileReader.h"
#include "TreeFile.h"
 
using namespace std;
 
DirectoryFileReader::DirectoryFileReader(Repository& repository, File file) :
     repository(repository), file(file), in(repository.createInputStream()), reader(0)
{}
 
DirectoryFileReader::~DirectoryFileReader()
{
  if (reader != 0) {
    reader->close();
    delete reader;
    reader = 0;
  }
}


void DirectoryFileReader::open()
{
  in.open(file);
  reader = new BufferedReader(&in);
  
  string header;
  reader->readLine(header);

  if (header != DIRECTORY_FILE_HEADER) {
    throw Exception(string("This does not look like a .shabackup file: ").append(file.path));
  }
}


TreeFileEntry DirectoryFileReader::next()
{
  string line;
  string emptyString;
  reader->readLine(line);

  if (line.size() >= 10) {
    return TreeFileEntry(line, emptyString);
  } else {
    return TreeFileEntry();
  }
}
