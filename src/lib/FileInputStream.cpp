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

#include <fcntl.h>
#include <iostream>
#include <unistd.h>

#include "FileInputStream.h"
#include "Exception.h"

using namespace std;

/*****************************************************************************\
 * FileInputStream                                                            |
 *****************************************************************************/
FileInputStream::FileInputStream(File& file)
{
  init(file.path);
}

FileInputStream::FileInputStream(const char* filename)
{
  string _filename(filename);
  init(_filename);
}

FileInputStream::FileInputStream(string& filename)
{
  init(filename);
}

FileInputStream::~FileInputStream()
{
  close();
}

/*****************************************************************************\
 * init                                                                       |
 *****************************************************************************/
void FileInputStream::init(string& filename)
{
#if defined(WIN32)

  handle = CreateFileA(filename.c_str(), GENERIC_READ,
      FILE_SHARE_READ, NULL, OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL, NULL);

  if (handle == INVALID_HANDLE_VALUE) {
    throw Exception::errnoToException(filename);
  }

#else

#if defined(__APPLE__)
  handle = ::open(filename.c_str(), O_RDONLY);
#else
  handle = ::open64(filename.c_str(), O_RDONLY | O_LARGEFILE);
#endif

  if (handle == -1) {
    throw Exception::errnoToException(filename);
  }

#endif
}

/*****************************************************************************\
 * read                                                                       |
 *****************************************************************************/
int FileInputStream::read()
{
  char b;
  long r;
  r = read(&b, 1);
  if (r == 1)
    return b & 0xff;
  else
    return -1;
}

/*****************************************************************************\
 * read                                                                       |
 *****************************************************************************/
int FileInputStream::read(char* b, int len)
{
#if defined(WIN32)

  DWORD r;

  if (!ReadFile(handle, b, len, &r, NULL)) {
    throw Exception::errnoToException();
  }

  if (r == 0)
  return -1;
  else
  return r;

#else

  int r = ::read(handle, b, len);
  if (r < 0)
    throw Exception::errnoToException();
  else if (r == 0)
    return -1;
  else
    return r;

#endif
}

/*****************************************************************************\
 * close                                                                      |
 *****************************************************************************/
void FileInputStream::close()
{
#if defined(WIN32)

  if (handle != INVALID_HANDLE_VALUE) {
    CloseHandle(handle);
    handle = INVALID_HANDLE_VALUE;
  }

#else

  if (handle != -1) {
    ::close(handle);
    handle = -1;
  }

#endif
}

/*****************************************************************************\
 * reset                                                                      |
 *****************************************************************************/
void FileInputStream::reset()
{
#if defined(WIN32)

  throw UnsupportedOperation("reset");

#else

  lseek(handle, 0, SEEK_SET);

#endif
}

