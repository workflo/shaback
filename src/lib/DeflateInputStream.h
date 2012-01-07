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

#ifndef SHABACK_DeflateInputStream_H
#define SHABACK_DeflateInputStream_H

#include <string.h>
#include <zlib.h>
#include "InputStream.h"

#define DEFLATE_CHUNK_SIZE (16 * 1024)

/**
 * An InputStream that performs Deflate data compression.
 *
 * @class DeflateInputStream
 */
class DeflateInputStream : public InputStream
{
public:
  DeflateInputStream(InputStream* in); 
  ~DeflateInputStream(); 

  int read();
  int read(char* b, int len);
  void close();

  void setBlocking(bool on) {};
  
protected:
  InputStream* in;
  z_stream zipStream;
  unsigned char readBuffer[DEFLATE_CHUNK_SIZE];
  int ret;
}; 
#endif// SHABACK_DeflateInputStream_H

