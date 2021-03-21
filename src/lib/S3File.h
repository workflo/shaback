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

#ifndef SHABACK_S3File_H
#define SHABACK_S3File_H

#include <string>
#include <vector>
#include <sys/stat.h>
#include <stdio.h>

#include "config.h"
#include "File.h"
#include "FileSystem.h"


class S3File : public File
{
  public:
    S3File(FileSystem* fileSystem, std::string path);
    S3File(FileSystem* fileSystem, File parent, std::string filename);
    ~S3File();

    void refresh();
    bool isFile();
    bool isDir();
    bool exists();
    bool mkdir();
    bool mkdirs();
    std::vector<File> listFiles(std::string pattern);
    std::string listFilesToString(std::string pattern, std::string delimiter);
    bool move(File& destination);
    File getParent();
    bool remove();

    std::string path;
    std::string fname;

    std::string getName();
    std::string getBasename(std::string suffix);

  protected:
    void canonicalize();

  private:
    bool initialized;
    bool fileExists;
    FileSystem* fileSystem;

    void assertInitialized();
};

#endif // SHABACK_S3File_H
