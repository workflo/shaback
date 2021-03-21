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

#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>

#include "S3File.h"
#include "Exception.h"
#include "FileInputStream.h"

using namespace std;

S3File::S3File(FileSystem* fileSystem, string path) :
  fileSystem(fileSystem), path(path), initialized(false)
{
  canonicalize();
  fname = path.substr(path.rfind("/") + 1);
}

S3File::S3File(FileSystem* fileSystem, File parent, string filename) : 
  fileSystem(fileSystem), initialized(false)
{
  path = parent.path;
  path.append("/").append(filename);
  canonicalize();
  fname = path.substr(path.rfind("/") + 1);
}

S3File::~S3File()
{
}

void S3File::assertInitialized()
{
  if (!initialized) {
    refresh();
  }
}

void S3File::refresh()
{
  initialized = true;
}

bool S3File::isFile()
{
  assertInitialized();
  return false;
}

bool S3File::isDir()
{
  assertInitialized();
  return false;
}

bool S3File::exists()
{
  assertInitialized();
  return fileExists && (isFile() || isDir());
}

bool S3File::mkdir()
{
  initialized = false;
#ifdef WIN32
  return (::_mkdir(this->path.c_str()) == 0);
#else
  return (::mkdir(this->path.c_str(), 0777) == 0);
#endif
}

bool S3File::mkdirs()
{
  if (this->path == ".") return false;

  initialized = false;
  File parent = getParent();

  if (!parent.isDir()) {
    parent.mkdirs();
  }

#ifdef WIN32
  return (::_mkdir(path.c_str()) == 0);
#else
  return (::mkdir(path.c_str(), 0777) == 0);
#endif
}

vector<File> S3File::listFiles(string p)
{
  vector<File> list;

  if (!isDir())
    return list;

  return list;
}

string S3File::listFilesToString(string p, string delimiter)
{
  vector<File> list = listFiles(p);
  string str;

  for (vector<File>::iterator it = list.begin(); it < list.end(); it++) {
    File file(*it);
    if (str.size() > 0) str.append(delimiter);
    str.append(file.getName());
  }

  return str;
}

bool S3File::move(File& destination)
{
  return (::rename(path.c_str(), destination.path.c_str()) == 0);
}

File S3File::getParent()
{
  int lastSlash = path.rfind("/");
  if (lastSlash == string::npos) {
    throw FileNotFoundException(string(path).append("/.."));
  }
//  cout << "getParent: " << path.substr(0, lastSlash)<<endl;
  return File(path.substr(0, lastSlash));
}

void S3File::canonicalize()
{
  // Remove all trailing slashes:
  while (path.size() > 1 && path.back() == '/') {
    path.pop_back();
  }

  // Remove duplicate slashes:
  int pos;
  while ((pos = path.find("//")) != string::npos) {
    path.erase(pos, 1);
  }
}

bool S3File::remove()
{
  int ret = ::remove(path.c_str());
  return (ret == 0);
}

string S3File::getName()
{
  return fname;
}

string S3File::getBasename(string suffix)
{
  int pos = fname.rfind(suffix);

  cout << "pos=" <<pos <<endl;
  if (pos == string::npos && pos != fname.size() - suffix.size()) {
    return fname;
  } else {
    return fname.substr(0, fname.size() - suffix.size());
  }
}
