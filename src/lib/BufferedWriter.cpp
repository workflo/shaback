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

#include <stdlib.h>
#include <string.h>

#include "BufferedWriter.h"
#include "Exception.h"

using namespace std;


/*****************************************************************************\
 * BufferedWriter                                                             |
 *****************************************************************************/
BufferedWriter::BufferedWriter(OutputStream* out, int bufferSize)
  : out(out)
{
  nChars = 0;
  this->bufferSize = bufferSize;
  buffer = (char*) malloc(bufferSize);
}


/*****************************************************************************\
 * ~BufferedWriter                                                            |
 *****************************************************************************/
BufferedWriter::~BufferedWriter()
{
  close();
  free(buffer);
}


/*****************************************************************************\
 * ensureOpen                                                                 |
 *****************************************************************************/
void BufferedWriter::ensureOpen()
{
  if (!out) throw IOException("Writer closed");
}


/*****************************************************************************\
 * write                                                                      |
 *****************************************************************************/
void BufferedWriter::write(const char* buf, int len)
{
  ensureOpen();

  for (int idx = 0; idx < len; idx++) {
    if (nChars >= bufferSize -1)
      flush();
    buffer[nChars++] = buf[idx];
  }
}

void BufferedWriter::write(int c)
{
  ensureOpen();

  if (nChars >= bufferSize -1)
    flush();
  buffer[nChars++] = (char) c & 0xffff;
}

void BufferedWriter::write(string& str)
{
  out->write(str.data(), str.size());
}

void BufferedWriter::write(const char* str)
{
  out->write(str, strlen(str));
}


/*****************************************************************************\
 * newLine                                                                    |
 *****************************************************************************/
void BufferedWriter::newLine()
{
#ifdef WIN32
  write("\r\n");
#else
  write("\n");
#endif
}


/*****************************************************************************\
 * close                                                                      |
 *****************************************************************************/
void BufferedWriter::close()
{
  if (out) {
    flush();
    out->close();
    out = 0;
  }
}


/*****************************************************************************\
 * flush                                                                      |
 *****************************************************************************/
void BufferedWriter::flush()
{
  ensureOpen();
  if (nChars < 1)
    return;

  out->write(buffer, nChars);
  nChars = 0;
}


