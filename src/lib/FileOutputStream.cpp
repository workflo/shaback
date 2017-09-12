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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "FileOutputStream.h"
#include "Exception.h"

using namespace std;

FileOutputStream::FileOutputStream(File& file)
{
  init(file.path);
}

FileOutputStream::FileOutputStream(const char *filename)
{
  string _filename(filename);
  init(_filename);
}

FileOutputStream::FileOutputStream(string& filename)
{
  init(filename);
}

FileOutputStream::~FileOutputStream()
{
  close();
}

/*****************************************************************************\
 * init                                                                       |
 *****************************************************************************/
void FileOutputStream::init(string& _filename)
{
  totalBytesWritten = 0;
  filename = _filename;

  
#if defined(WIN32)

  handle = CreateFileA(filename.c_str(), GENERIC_WRITE,
      0, NULL, CREATE_ALWAYS,
      FILE_ATTRIBUTE_NORMAL, NULL);

  if (handle == INVALID_HANDLE_VALUE) {
    throw Exception::errnoToException(filename);
  }

#else
#if defined(__APPLE__)
  handle = ::open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
#else
  handle = ::open64(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
#endif

  if (handle == -1) {
    throw Exception::errnoToException(filename);
  }

#endif
}

/*****************************************************************************\
 * write                                                                      |
 *****************************************************************************/
void FileOutputStream::write(int b)
{
  char c = (char) b;
  write(&c, 1);
}

/*****************************************************************************\
 * write                                                                      |
 *****************************************************************************/
void FileOutputStream::write(const char* b, int len)
{
  //   if (b == 0)
  //     throw new NullPointerException();
  //   else if (len < 0)
  //     throw new IndexOutOfBoundsException();
  //   else if (len == 0)
  //     return;

#if defined(WIN32)

  DWORD written;

  if (!WriteFile(handle, b, len, &written, NULL)) {
    throw Exception::errnoToException();
  }

#else

  if (::write(handle, b, len) != len) {
    throw Exception::errnoToException();
  }

#endif

  totalBytesWritten += len;
}

/*****************************************************************************\
 * close                                                                      |
 *****************************************************************************/
void FileOutputStream::close()
{
#if defined(WIN32)

  if (handle != INVALID_HANDLE_VALUE) {
    CloseHandle(handle);
    handle = INVALID_HANDLE_VALUE;
  }

#else

  if (handle != -1) {
    if (::close(handle) != 0) {
      throw Exception::errnoToException();
    }
    handle = -1;

    if (File(filename).getSize() != totalBytesWritten) {
      char buf[50];
#ifdef __APPLE__
      sprintf(buf, "%lld", totalBytesWritten);
#else
      sprintf(buf, "%lu", totalBytesWritten);
#endif
      throw IOException(string("IO ERROR: Written output file has unexpected size: ") + filename + " should be " + buf + " bytes");
    }
  }

#endif
}
