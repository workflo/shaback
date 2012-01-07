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
#include "InputStream.h"
#include "Exception.h"

using namespace std;

const long InputStream::SKIP_BUFFER_SIZE = 2048;
char* InputStream::skipBuffer = 0;

InputStream::~InputStream()
{
}

int InputStream::read(char* b, int len)
{
  int c = read();
  if (c == -1) {
    return -1;
  }
  b[0] = (char) c;

  int r = 1;
  try {
    for (; r < len; r++) {
      c = read();
      if (c == -1)
        break;
      if (b != 0)
        b[r] = (char) c;
    }
  } catch (IOException ee) {
  }

  return r;
}

bool InputStream::readLine(string& str)
{
  int c;

  str.clear();

  c = read();
  if (c < 0)
    return false;

  for (;;) {
    if (c == '\n') {
      return true;
    } else if (c != '\r') {
      str.append((char*) &c, 1);
    }
    c = read();

    if (c < 0)
      return true;
  }
}

#define INPUTSTREAM_READ_BUFFER_LEN 8192

bool InputStream::readAll(string& str)
{
  str.clear();
  char buf[INPUTSTREAM_READ_BUFFER_LEN];

  int bytesRead = read(buf, INPUTSTREAM_READ_BUFFER_LEN);
  if (bytesRead == -1)
    return false;

  while (true) {
    str.append(buf, bytesRead);
    bytesRead = read(buf, INPUTSTREAM_READ_BUFFER_LEN);
    if (bytesRead == -1)
      break;
  }

  return true;
}

void InputStream::close()
{
}

void InputStream::reset()
{
  throw IOException("reset() not supported");
}

void InputStream::copyTo(OutputStream& destination, int maxBytes)
{
  char buffer[8192];
  int bytesToRead = maxBytes;

  for (;;) {
    int bytesRead = read(buffer, maxBytes == -1 ? 8192 : min(8192, bytesToRead));
    if (bytesRead <= 0)
      break;
    destination.write(buffer, bytesRead);
    bytesToRead -= bytesRead;
  }
}

void InputStream::copyTo(OutputStream& destination)
{
  copyTo(destination, -1);
}
