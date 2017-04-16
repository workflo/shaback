/*
 * shaback - A hash digest based backup tool.
 * Copyright (C) 2017 Florian Wolff (florian@donuz.de)
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
#include "ZStdOutputStream.h"

#if defined(ZSTD_FOUND)

#include "Exception.h"

using namespace std;


ZStdOutputStream::ZStdOutputStream(OutputStream* out, int compressionLevel)
  : out(out)
{
  zipStream = ZSTD_createCStream();

  if (zipStream == 0) {
    throw ZStdException("ZSTD_createCStream failed");
  }

  size_t const initResult = ZSTD_initCStream(zipStream, compressionLevel);

  if (ZSTD_isError(initResult)) {
    throw ZStdException("ZSTD_initCStream failed", ZSTD_getErrorName(initResult));
  } 

  outBuffer.size = ZSTD_CStreamOutSize();
  outBuffer.pos = 0;
  outBuffer.dst = (char*) malloc(outBuffer.size);

  inBuffer.size = ZSTD_CStreamInSize();
  inBuffer.pos = 0;
}

ZStdOutputStream::~ZStdOutputStream()
{
  close();
  ZSTD_freeCStream(zipStream);
  zipStream = 0;
  free(outBuffer.dst);
}


void ZStdOutputStream::write(int b)
{
  char c = (char) b;
  write(&c, 1);
}


void ZStdOutputStream::write(const char* b, int len)
{
  while (len > 0) {
    size_t nextChunkLen = min((size_t) len, inBuffer.size);
    writeChunk(b, nextChunkLen);
    b += nextChunkLen;
    len -= nextChunkLen;
  }
}

void ZStdOutputStream::writeChunk(const char* b, size_t len)
{
  size_t toRead = len;

  ZSTD_inBuffer input = { b, len, 0 };
  while (input.pos < input.size) {
    outBuffer.pos = 0;
    toRead = ZSTD_compressStream(zipStream, &outBuffer , &input);

    if (ZSTD_isError(toRead)) { 
       throw ZStdException("ZSTD_compressStream failed", ZSTD_getErrorName(toRead));
    }

    out->write((char*)outBuffer.dst, outBuffer.pos);
  }
}


void ZStdOutputStream::finish()
{
  outBuffer.pos = 0;
  size_t const remainingToFlush = ZSTD_endStream(zipStream, &outBuffer);
  if (remainingToFlush) {
    throw ZStdException("ZSTD_endStream: not fully flushed"); 
  }

  out->write((char*)outBuffer.dst, outBuffer.pos);
}


void ZStdOutputStream::close()
{
  if (out) {
    finish();
    out->close();
    out = 0;
  }
}

#endif
