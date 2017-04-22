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

  inbufferSize = ZSTD_DStreamInSize();
  inBuffer.size = 0;
  inBuffer.pos = 0;
  inBuffer.src = (char*) malloc(inbufferSize);
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
  if (inBuffer.pos >= inBuffer.size) {
    int read = in->read((char*)inBuffer.src, inbufferSize);
    if (read <= 0) return -1;
    inBuffer.pos = 0;
    inBuffer.size = read;
  }

  ZSTD_outBuffer output = { b, len, 0 };

  while (inBuffer.pos < inBuffer.size && output.pos < output.size) {
    size_t ret = ZSTD_decompressStream(zipStream, &output, &inBuffer);

    if (ZSTD_isError(ret)) { 
       throw ZStdException("ZSTD_decompressStream failed", ZSTD_getErrorName(ret));
    }
  }
  return output.pos;
}


void ZStdInputStream::close()
{
  if (in) {
    in->close();
    in = 0;
  }
}

#endif
