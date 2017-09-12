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
#if !defined(SHABACK_ZStdInputStream_H) && defined(ZSTD_FOUND)
#define SHABACK_ZStdInputStream_H

#include <string.h>
#include <zstd.h>
#include "InputStream.h"

/**
 * An InputStream that performs ZStandard data decompression.
 *
 * @class ZStdInputStream
 */
class ZStdInputStream: public InputStream
{
  public:
    ZStdInputStream(InputStream* in);
    ~ZStdInputStream();

    int read();
    int read(char* b, int len);
    void close();

    void setBlocking(bool on)
    {
    }
    ;

  protected:
    InputStream* in;
    ZSTD_DStream* zipStream;
    size_t inbufferSize;

    ZSTD_inBuffer inBuffer;
};
#endif // SHABACK_ZStdInputStream_H
