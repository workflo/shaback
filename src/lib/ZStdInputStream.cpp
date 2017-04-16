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
#include "ZStdInputStream.h"

#if defined(ZSTD_FOUND)

#include "Exception.h"

using namespace std;


ZStdInputStream::ZStdInputStream(InputStream* in)
  : in(in)
{
  zipStream = ZSTD_createDStream();

  if (zipStream == 0) {
    throw ZStdException("ZSTD_createDStream failed");
  }

  size_t const initResult = ZSTD_initDStream(zipStream);

  if (ZSTD_isError(initResult)) {
    throw ZStdException("ZSTD_initDStream failed", ZSTD_getErrorName(initResult));
  } 

  inBuffer.size = ZSTD_DStreamInSize();
  inBuffer.pos = 0;
  inBuffer.src = (char*) malloc(inBuffer.size);

  outBuffer.size = ZSTD_DStreamOutSize();
  outBuffer.pos = 0;
}

ZStdInputStream::~ZStdInputStream()
{
  close();
  ZSTD_freeDStream(zipStream);
  zipStream = 0;
  free((void*) inBuffer.src);
}


int ZStdInputStream::read()
{
  throw UnsupportedOperation("read()");
}


int ZStdInputStream::read(char* b, int len)
{
  size_t read = 0;

  while (len > 0) {
    size_t nextChunkLen = min((size_t) len, inBuffer.size);
    read = readChunk(b, nextChunkLen);
    b += nextChunkLen;
    len -= nextChunkLen;
  }
}

size_t ZStdInputStream::readChunk(char* b, size_t len)
{
  int bytesRead = in->read((char*)inBuffer.src, len);

  if (bytesRead == -1) return -1;


  size_t toRead = len;

  ZSTD_inBuffer input = { b, len, 0 };
  while (input.pos < input.size) {
    outBuffer.pos = 0;
    toRead = ZSTD_decompressStream(zipStream, &outBuffer , &input);

    if (ZSTD_isError(toRead)) { 
       throw ZStdException("ZSTD_compressStream failed", ZSTD_getErrorName(toRead));
    }
  }
}


void ZStdInputStream::close()
{
  if (in) {
    in->close();
    in = 0;
  }
}

#endif
