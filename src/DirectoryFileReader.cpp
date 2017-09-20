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
     repository(repository), file(file)
{}
 
DirectoryFileReader::~DirectoryFileReader()
{}


void DirectoryFileReader::open()
{
  ShabackInputStream in = repository.createInputStream();
  in.open(file);
  BufferedReader reader(&in);
  
  string header;
  reader.readLine(header);

  if (header != DIRECTORY_FILE_HEADER) {
    throw Exception(string("This does not look like a .shabackup file: ").append(file.path));
  }
  //   vector<TreeFileEntry> list;
  //   int from = 0;
  //   int until;
  
  //   if ((until = content.find('\n', from)) == string::npos) {
  //     if (config.verbose) {
  //       cerr << config.color_error;
  //       cerr << "Missing header line in index file " << hashValueToFile(treeId).path << ":" << endl;
  //       cerr << config.color_low;
  //       cerr << content.substr(0, 200) << (content.size() > 200 ? "..." : "") << endl;
  //       cerr << config.color_default;
  //     }
  //     throw InvalidTreeFile(string("Missing header line in index ") + treeId);
  //   }
  //   string header = content.substr(from, until - from);
  //   if (header != TREEFILE_HEADER)
  //     throw InvalidTreeFile("Unexpected header line in tree file");
  //   from = until + 1;
  
  //   if ((until = content.find('\n', from)) == string::npos)
  //     throw InvalidTreeFile("Missing parent directory line");
  //   string parentDir = content.substr(from, until - from);
  //   from = until + 1;
  
  //   while ((until = content.find('\n', from)) != string::npos) {
  //     string line = content.substr(from, until - from);
  //     list.push_back(TreeFileEntry(line, parentDir));
  //     from = until + 1;
  //   }
  
  //   return list;
}
