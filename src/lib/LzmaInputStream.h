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

#include "config.h"
#if !defined(SHABACK_LzmaInputStream_H) && defined(LZMA_FOUND)
#define SHABACK_LzmaInputStream_H

#include <string.h>
#include <lzma.h>
#include "InputStream.h"

#define LZMA_CHUNK_SIZE (16 * 1024)

/**
 * An InputStream that performs Lzmaip data compression.
 *
 * @class LzmaInputStream
 */
class LzmaInputStream: public InputStream
{
  public:
    LzmaInputStream(InputStream* in);
    ~LzmaInputStream();

    int read();
    int read(char* b, int len);
    void close();

    void setBlocking(bool on)
    {
    }
    ;

  protected:
    InputStream* in;
    lzma_stream zipStream;
    char readBuffer[LZMA_CHUNK_SIZE];
    int ret;
};
#endif // SHABACK_LzmaInputStream_H
