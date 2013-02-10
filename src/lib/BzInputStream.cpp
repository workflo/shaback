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

#include "BzInputStream.h"
#include "Exception.h"

using namespace std;


BzInputStream::BzInputStream(InputStream* in)
  : in(in)
{
  zipStream.bzalloc = 0;
  zipStream.bzfree = 0;
  zipStream.opaque = 0;

  if ((ret = BZ2_bzDecompressInit(&zipStream, 0, 0)) != BZ_OK) {
    throw BzException("BZ2_bzDecompressInit failed", ret);
  }

  zipStream.avail_in = 0;
}

BzInputStream::~BzInputStream()
{
  close();
}


int BzInputStream::read()
{
  throw UnsupportedOperation("read()");
}


int BzInputStream::read(char* b, int len)
{
  if (zipStream.avail_in == 0) {
    int bytesRead = in->read(readBuffer, min(len, BZ_CHUNK_SIZE));

    zipStream.avail_in = bytesRead;
    zipStream.next_in = readBuffer;

    if (bytesRead == -1) return -1;
  }
    
  zipStream.avail_out = len;
  zipStream.next_out = b;

  ret = BZ2_bzDecompress(&zipStream);

  
  if (ret == BZ_STREAM_END && zipStream.avail_out == len) {
    return -1;
  } else if (ret < 0) {
    throw BzException("BZ2_bzDecompress failed", ret);
  }

  return min(len, BZ_CHUNK_SIZE) - zipStream.avail_out;
}


void BzInputStream::close()
{
  if (in) {
    BZ2_bzDecompressEnd(&zipStream);
    in->close();
    in = 0;
    ret = BZ_STREAM_END;
  }
}

