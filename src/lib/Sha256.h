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
#if !defined(SHABACK_Sha256_H) && defined(OPENSSL_FOUND)
#define SHABACK_Sha256_H

#include <string>
#include <openssl/sha.h>
#include "Digest.h"

class Sha256: public Digest
{
  public:
    Sha256();
    ~Sha256();

    void reset();

    void update(const unsigned char* data, unsigned long len);
    void update(std::string& data);

    std::string toString();
    const unsigned char* toBytes();
    void finalize();

  private:
    std::string hexStr;
    SHA256_CTX *ctx;
    unsigned char bytes[SHA256_DIGEST_LENGTH];
};

#endif // SHABACK_Sha256_H
