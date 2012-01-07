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

#include "DeflateInputStream.h"
#include "Exception.h"

using namespace std;


DeflateInputStream::DeflateInputStream(InputStream* in)
  : in(in)
{
  zipStream.zalloc = Z_NULL;
  zipStream.zfree = Z_NULL;
  zipStream.opaque = Z_NULL;

  if ((ret = inflateInit(&zipStream)) != Z_OK) {
    throw DeflateException(string("inflateInit failed: ").append(zError(ret)));
  }
  zipStream.avail_in = 0;
}

DeflateInputStream::~DeflateInputStream()
{
  close();
}


int DeflateInputStream::read()
{
  throw UnsupportedOperation("read()");
}


int DeflateInputStream::read(char* b, int len)
{
  if (zipStream.avail_in == 0) {
    int bytesRead = in->read((char*) readBuffer, min(len, DEFLATE_CHUNK_SIZE));

    zipStream.avail_in = bytesRead;
    zipStream.next_in = readBuffer;

    if (bytesRead == -1) return -1;
  }
    
  zipStream.avail_out = len;
  zipStream.next_out = (unsigned char*) b;

  ret = inflate(&zipStream, Z_NO_FLUSH);

  
  if (ret == Z_STREAM_END && zipStream.avail_out == len) {
    return -1;
  } else if (ret < 0) {
    throw DeflateException(string("inflate failed: ").append(zError(ret)));
  }

  return min(len, DEFLATE_CHUNK_SIZE) - zipStream.avail_out;
}


void DeflateInputStream::close()
{
  if (ret != Z_STREAM_END) {
    inflateEnd(&zipStream);
    in->close();
    ret = Z_STREAM_END;
  }
}

