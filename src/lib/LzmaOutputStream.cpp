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
#include "LzmaOutputStream.h"

#if defined(LZMA_FOUND)

#include "Exception.h"

using namespace std;


LzmaOutputStream::LzmaOutputStream(OutputStream* out, int compressionLevel)
  : out(out)
{
  lzma_stream tmp = LZMA_STREAM_INIT;
  memcpy(&zipStream, &tmp, sizeof(lzma_stream));

  if ((ret = lzma_easy_encoder(&zipStream, compressionLevel, LZMA_CHECK_CRC64)) != LZMA_OK) {
    throw LzmaException("lzma_easy_encoder failed", ret);
  }

  outputBuffer = (char*) malloc(LZMA_CHUNK_SIZE);
}

LzmaOutputStream::~LzmaOutputStream()
{
  close();
  free(outputBuffer);
}


void LzmaOutputStream::write(int b)
{
  char c = (char) b;
  write(&c, 1);
}


void LzmaOutputStream::write(const char* b, int len)
{
  if (len <= 0) return;

  zipStream.avail_in = len;
  zipStream.next_in = (uint8_t*) b;
    
  do {
    zipStream.avail_out = LZMA_CHUNK_SIZE;
    zipStream.next_out = (uint8_t*) outputBuffer;

    ret = lzma_code(&zipStream, LZMA_RUN);
    if (ret < 0) {
      throw LzmaException("lzma_code failed", ret);
    }
    out->write((const char*) outputBuffer, LZMA_CHUNK_SIZE - zipStream.avail_out);
  } while (zipStream.avail_out == 0 && zipStream.avail_in > 0);
}


void LzmaOutputStream::finish()
{
  if (ret == LZMA_STREAM_END) return;

  char inBuf[1];

  zipStream.avail_in = 0;
  zipStream.next_in = (uint8_t*) inBuf;
    
  do {
    zipStream.avail_out = LZMA_CHUNK_SIZE;
    zipStream.next_out = (uint8_t*) outputBuffer;
     
    ret = lzma_code(&zipStream, LZMA_FINISH);
    if (ret < 0) {
      throw LzmaException("lzma_code failed", ret);
    }
    out->write((const char*) outputBuffer, LZMA_CHUNK_SIZE - zipStream.avail_out);
  } while (zipStream.avail_out == 0);

  lzma_end(&zipStream);
}


void LzmaOutputStream::close()
{
  if (ret != LZMA_STREAM_END) {
    finish();
    out->close();
    ret = LZMA_STREAM_END;
  }
}

#endif
