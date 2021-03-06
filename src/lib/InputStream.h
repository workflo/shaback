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

#ifndef SHABACK_InputStream_H
#define SHABACK_InputStream_H

#include <string>

#include "OutputStream.h"

/**
 * This abstract class forms the base of the hierarchy of classes that read
 * input as a stream of bytes.
 *
 * It provides a common set of methods for
 * reading bytes from streams. Subclasses implement and extend these
 * methods to read bytes from a particular input source such as a file
 * or network connection.
 *
 * @class InputStream
 */
class InputStream
{
  public:

    virtual ~InputStream();

    /**
     * Reads the next \c jbyte from the stream and returns it as an integer value in the range
     * <tt>0..255</tt>.
     *
     * If the end of the stream is reached, <tt>-1</tt> is returned.
     * <tt>read</tt> blocks if no data is available but the end of the stream is not
     * reached yet. If non-blocking mode is actived (see \c setBlocking) \c -2
     * is returned if there is no data available.
     *
     * @returns Next \c jbyte from stream or \c -1 if end of stream
     *          reached or \c -2 if non-blocking mode and no data available
     */
    virtual int read() = 0;

    /**
     * Tries to read \a len number of bytes.
     *
     * @return \c -1 if not even a single byte could be read, the
     *         exact number of bytes read otherwise.
     */
    virtual int read(char* b, int len);

    /**
     * Reads a full line of text. I.e. all characters up to the next newline.
     */
    virtual bool readLine(std::string& str);

    virtual bool readAll(std::string& str);

    /**
     * Closes this input stream.
     */
    virtual void close();

    /**
     * Reset the read cursor to next the most recent mark.
     */
    virtual void reset();

    /**
     * Copies this InputStream to the specified OutputStream.
     * An internal 8kB buffer is used.
     */
    void copyTo(OutputStream& destination);

    /**
     * Copies this InputStream to the specified OutputStream.
     * An internal 8kB buffer is used.
     * Reads/Writes up to \a maxBytes number of bytes.
     */
    void copyTo(OutputStream& destination, int maxBytes);

    static const long SKIP_BUFFER_SIZE;

    static char* skipBuffer;
};
#endif // SHABACK_InputStream_H
