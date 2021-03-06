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
#if !defined(SHABACK_LzmaOutputStream_H) && defined(LZMA_FOUND)
#define SHABACK_LzmaOutputStream_H

#include <string.h>
#include <lzma.h>
#include "OutputStream.h"

#define LZMA_CHUNK_SIZE (16 * 1024)

/**
 * An OutputStream that performs LZMA data compression.
 *
 * @class LzmaOutputStream
 */
class LzmaOutputStream: public OutputStream
{
  public:
    LzmaOutputStream(OutputStream* out, int compressionLevel = 5);
    ~LzmaOutputStream();

    void write(int b);
    void write(const char* b, int len);
    void finish();
    void close();

  protected:
    OutputStream* out;
    lzma_stream zipStream;
    char* outputBuffer;
    lzma_ret ret;
};
#endif // SHABACK_LzmaOutputStream_H
