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

#include "BufferedReader.h"
#include "Exception.h"

using namespace std;

/*****************************************************************************\
 * BufferedReader                                                             |
 *****************************************************************************/
BufferedReader::BufferedReader(InputStream* in, int bufferSize) :
  in(in), bufferSize(bufferSize)
{
  nextChar = 0;
  nChars = 0;
  buffer = (char*) malloc(bufferSize);
}

/*****************************************************************************\
 * ~BufferedReader                                                            |
 *****************************************************************************/
BufferedReader::~BufferedReader()
{
  free(buffer);
}


/*****************************************************************************\
 * fill                                                                       |
 *****************************************************************************/
void BufferedReader::fill()
{
  if (nextChar < nChars)
    return;

  nextChar = 0;
  nChars = in->read(buffer, bufferSize);
}

/*****************************************************************************\
 * ensureOpen                                                                 |
 *****************************************************************************/
void BufferedReader::ensureOpen()
{
  if (!in) throw IOException("Reader closed");
}

/*****************************************************************************\
 * read                                                                       |
 *****************************************************************************/
int BufferedReader::read()
{
  ensureOpen();
  if (nextChar >= nChars) {
    fill();
    if (nextChar >= nChars)
      return -1;
  }

  return buffer[nextChar++];
}

int BufferedReader::read(char* b, int len)
{
  ensureOpen();

  int c = read();
  if (c < 0) {
    return -1;
  }
  b[0] = (unsigned char) c;

  //try {
  int r = 1;
  for (; r < len; r++) {
    c = read();
    if (c == -1)
      break;
    else
      b[r] = (unsigned char) c;
  }
  //} 
  //catch (IOException ee) {}

  return r;
}


/*****************************************************************************\
 * close                                                                      |
 *****************************************************************************/
void BufferedReader::close()
{
  if (in) {
    in->close();
    in = 0;
  }
}
