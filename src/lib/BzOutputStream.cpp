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

#include "BzOutputStream.h"
#include "Exception.h"

using namespace std;


BzOutputStream::BzOutputStream(OutputStream* out)
  : out(out)
{
  zipStream.bzalloc = 0;
  zipStream.bzfree = 0;
  zipStream.opaque = 0;

  if ((ret = BZ2_bzCompressInit(&zipStream, BZ_CHUNK_SIZE_100K, 3, 1)) != BZ_OK) {
    // TODO: BzOutputStream: Exception
    //throw BzException(string("BZ2_bzCompressInit failed: ").append(BZ2_bzerror(0, ret)));
  }
}

BzOutputStream::~BzOutputStream()
{
  close();
}


void BzOutputStream::write(int b)
{
  char c = (char) b;
  write(&c, 1);
}


void BzOutputStream::write(const char* b, int len)
{
  if (len <= 0) return;

  zipStream.avail_in = len;
  zipStream.next_in = (char*) b;
    
  do {
    zipStream.avail_out = BZ_CHUNK_SIZE;
    zipStream.next_out = outputBuffer;
      
    ret = BZ2_bzCompress(&zipStream, BZ_RUN);
    if (ret < 0) {
      // TODO: BzOutputStream: Exception
      //throw BzException(string("Bz failed: ").append(zError(ret)));
    }
    out->write((const char*) outputBuffer, BZ_CHUNK_SIZE - zipStream.avail_out);
  } while (zipStream.avail_out == 0);
}


void BzOutputStream::finish()
{
  if (ret == BZ_STREAM_END) return;

  char inBuf[1];

  zipStream.avail_in = 0;
  zipStream.next_in = inBuf;
    
  do {
    zipStream.avail_out = BZ_CHUNK_SIZE;
    zipStream.next_out = outputBuffer;
     
    ret = BZ2_bzCompress(&zipStream, BZ_FINISH);
    if (ret < 0) {
      // TODO: BzOutputStream: Exception
      //throw BzException(string("Bz failed: ").append(zError(ret)));
    }
    out->write((const char*) outputBuffer, BZ_CHUNK_SIZE - zipStream.avail_out);
  } while (zipStream.avail_out == 0);

  BZ2_bzCompressEnd(&zipStream);
}


void BzOutputStream::close()
{
  if (ret != BZ_STREAM_END) {
    finish();
    out->close();
    ret = BZ_STREAM_END;
  }
}

