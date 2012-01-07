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

#ifndef SHABACK_BufferedReader_H
#define SHABACK_BufferedReader_H

#include <string>
#include "InputStream.h"

/**
 * \c BufferedReader provides a \c Reader that uses an internal
 * buffer to improve performance when subsequently reading single
 * characters.
 *
 * It also provides the \c readString() method that reads a full line
 * of text (until the next newline character).
 *
 * @class BufferedReader
 */
class BufferedReader: public InputStream
{
  public:

    /**
     * @brief Creates a new \c BufferedReader from a given \c Reader object.
     * Uses the given \c bufferSize to allocate the input buffer.
     *
     * @param reader The \c Reader to be buffered.
     * @param bufferSize Buffer size in bytes, default is 8kB.
     */
    BufferedReader(InputStream* in, int bufferSize = 8192);

    /**
     * @brief The destructor.
     */
    ~BufferedReader();

    void close();

    virtual int read();

    virtual int read(char* buf, int len);

  private:

    /** @brief The current buffer size */
    int bufferSize;

    InputStream* in;
    char* buffer;

    int nChars;
    int nextChar;

    /**
     * @throws IOException If the reader is closed
     */
    void ensureOpen();

    /**
     * @brief (Re)Fills the input buffer.
     */
    void fill();
};
#endif // SHABACK_BufferedReader_H
