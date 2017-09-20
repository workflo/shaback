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

#ifndef SHABACK_Digest_H
#define SHABACK_Digest_H

#include <string>

class Digest
{
  public:
    virtual void reset() = 0;

    virtual void update(const unsigned char* data, unsigned long len) = 0;
    virtual void update(std::string& data) = 0;

    virtual std::string toString() = 0;
    virtual const unsigned char* toBytes() = 0;
    virtual void finalize() = 0;

    // static bool looksLikeDigest(std::string& str);

  protected:
    std::string hexStr;

    static char HEX_CHARS[];
};

#endif // SHABACK_Digest_H
