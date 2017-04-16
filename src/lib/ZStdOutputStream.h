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
#if !defined(SHABACK_ZStdOutputStream_H) && defined(ZSTD_FOUND)
#define SHABACK_ZStdOutputStream_H

#include <string.h>
#include <zstd.h>
#include "OutputStream.h"

/**
 * An OutputStream that performs ZStandard data compression.
 *
 * @class ZStdOutputStream
 */
class ZStdOutputStream: public OutputStream
{
  public:
    ZStdOutputStream(OutputStream* out, int compressionLevel = 5);
    ~ZStdOutputStream();

    void write(int b);
    void write(const char* b, int len);
    void finish();
    void close();

  protected:
    void writeChunk(const char* b, size_t len);

    OutputStream* out;
    ZSTD_CStream* zipStream;

    ZSTD_inBuffer inBuffer;
    ZSTD_outBuffer outBuffer;

    // void* buffIn;
    // void* buffOut;
};
#endif // SHABACK_ZStdOutputStream_H
