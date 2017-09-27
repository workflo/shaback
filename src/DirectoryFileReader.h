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

 #ifndef SHABACK_DirectoryFileReader_H
 #define SHABACK_DirectoryFileReader_H
 
 #include "Repository.h"
 #include "lib/File.h"
 #include "lib/BufferedReader.h"
 
 class DirectoryFileReader
 {
   public:
     DirectoryFileReader(Repository& repository, File file);
     ~DirectoryFileReader();

     TreeFileEntry next();

     void open(); 
 
   protected:
    File file;
    Repository& repository;
    ShabackInputStream in;
    BufferedReader* reader;
 };
 
 #endif // SHABACK_DirectoryFileReader_H
 