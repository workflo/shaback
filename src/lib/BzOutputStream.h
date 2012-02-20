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

#ifndef SHABACK_BzOutputStream_H
#define SHABACK_BzOutputStream_H

#include <string.h>
#include <bzlib.h>
#include "OutputStream.h"

#define BZ_CHUNK_SIZE_100K (2)
#define BZ_CHUNK_SIZE (BZ_CHUNK_SIZE_100K * 1024 * 100)

/**
 * An OutputStream that performs BZ data compression.
 *
 * @class BzOutputStream
 */
class BzOutputStream: public OutputStream
{
  public:
    BzOutputStream(OutputStream* out);
    ~BzOutputStream();

    void write(int b);
    void write(const char* b, int len);
    void finish();
    void close();

  protected:
    OutputStream* out;
    bz_stream zipStream;
    char outputBuffer[BZ_CHUNK_SIZE];
    int ret;
};
#endif// SHABACK_BzOutputStream_H
