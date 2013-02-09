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

#include "DeflateOutputStream.h"
#include "Exception.h"

using namespace std;


DeflateOutputStream::DeflateOutputStream(OutputStream* out)
  : out(out)
{
  zipStream.zalloc = Z_NULL;
  zipStream.zfree = Z_NULL;
  zipStream.opaque = Z_NULL;

  if ((ret = deflateInit(&zipStream, Z_DEFAULT_COMPRESSION)) != Z_OK) {
    throw DeflateException(string("deflateInit failed: ").append(zError(ret)));
  }
}

DeflateOutputStream::~DeflateOutputStream()
{
  close();
}


void DeflateOutputStream::write(int b)
{
  char c = (char) b;
  write(&c, 1);
}


void DeflateOutputStream::write(const char* b, int len)
{
  if (len <= 0) return;

  zipStream.avail_in = len;
  zipStream.next_in = (unsigned char*) b;
    
  do {
    zipStream.avail_out = DEFLATE_CHUNK_SIZE;
    zipStream.next_out = outputBuffer;
      
    ret = deflate(&zipStream, Z_NO_FLUSH);
    if (ret < 0) {
      throw DeflateException(string("deflate failed: ").append(zError(ret)));
    }
    out->write((const char*) outputBuffer, DEFLATE_CHUNK_SIZE - zipStream.avail_out);
  } while (zipStream.avail_out == 0);
}


void DeflateOutputStream::finish()
{
  if (ret == Z_STREAM_END) return;

  unsigned char inBuf[1];

  zipStream.avail_in = 0;
  zipStream.next_in = inBuf;
    
  do {
    zipStream.avail_out = DEFLATE_CHUNK_SIZE;
    zipStream.next_out = outputBuffer;
     
    ret = deflate(&zipStream, Z_FINISH);
    if (ret < 0) {
      throw DeflateException(string("deflate failed: ").append(zError(ret)));
    }
    out->write((const char*) outputBuffer, DEFLATE_CHUNK_SIZE - zipStream.avail_out);
  } while (zipStream.avail_out == 0);

  deflateEnd(&zipStream);
}


void DeflateOutputStream::close()
{
  if (out) {
    finish();
    out->close();
    out = 0;
    ret = Z_STREAM_END;
  }
}

