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

#include "LzmaInputStream.h"
#include "Exception.h"

using namespace std;


LzmaInputStream::LzmaInputStream(InputStream* in)
  : in(in)
{
  lzma_stream tmp = LZMA_STREAM_INIT;
  memcpy(&zipStream, &tmp, sizeof(lzma_stream));

  if ((ret = lzma_stream_decoder(&zipStream, UINT64_MAX, LZMA_TELL_NO_CHECK)) != LZMA_OK) {
    throw LzmaException("lzma_stream_decoder failed", ret);
  }

  zipStream.avail_in = 0;
}

LzmaInputStream::~LzmaInputStream()
{
  close();
}


int LzmaInputStream::read()
{
  throw UnsupportedOperation("read()");
}


int LzmaInputStream::read(char* b, int len)
{
  if (zipStream.avail_in == 0) {
    int bytesRead = in->read(readBuffer, min(len, LZMA_CHUNK_SIZE));

    zipStream.avail_in = bytesRead;
    zipStream.next_in = (uint8_t*) readBuffer;

    if (bytesRead == -1) return -1;
  }
    
  zipStream.avail_out = len;
  zipStream.next_out = (uint8_t*) b;

  ret = lzma_code(&zipStream, LZMA_RUN);

  
  if (ret == LZMA_STREAM_END && zipStream.avail_out == len) {
    return -1;
  } else if (ret < 0) {
    throw LzmaException("Lzma2_LzmaDecompress failed", ret);
  }

  return min(len, LZMA_CHUNK_SIZE) - zipStream.avail_out;
}


void LzmaInputStream::close()
{
  if (ret != LZMA_STREAM_END) {
    lzma_end(&zipStream);
    in->close();
    ret = LZMA_STREAM_END;
  }
}

