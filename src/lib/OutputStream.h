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

#ifndef SHABACK_OutputStream_H
#define SHABACK_OutputStream_H

#include <string>

/**
 * This abstract class forms the base of the hierarchy of classes that 
 * write output as a stream of bytes.
 *
 * It provides a common set of methods
 * for writing bytes to stream. Subclasses implement and/or extend these
 * methods to write bytes in a particular manner or to a particular 
 * destination such as a file on disk or network connection.
 *
 * @class OutputStream
 */
class OutputStream
{
  public:
    virtual ~OutputStream() = 0;

    /**
     * Writes the specified byte value to this output stream.
     *
     * @param b The byte value to be written. Only the lower 8 bits
     *          are written, all remaining bits are ignored.
     */
    virtual void write(int b) = 0;

    /**
     * Writes a number of bytes to the output stream.
     *
     * @param b The byte array to be written.
     * @param len Number of bytes to be written.
     */
    virtual void write(const char* b, int len);

    virtual void write(std::string& str);

    /**
     * Closes the output stream.
     */
    virtual void close() = 0;
};
#endif // SHABACK_OutputStream_H
