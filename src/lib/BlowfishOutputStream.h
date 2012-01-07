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

#ifndef SHABACK_BlowfishOutputStream_H
#define SHABACK_BlowfishOutputStream_H

#include <string.h>
#include <openssl/evp.h>
#include <openssl/blowfish.h>
#include "OutputStream.h"

#define SHABACK_IV "SHABACK2"
#define BLOWFISH_CHUNK_SIZE (BF_BLOCK * 1024 /8)

class BlowfishOutputStream: public OutputStream
{
  public:
    BlowfishOutputStream(std::string& password, OutputStream* out);
    ~BlowfishOutputStream();

    void write(int b);
    void write(const char* b, int len);
    void finish();
    void close();

  protected:
    OutputStream* out;
    unsigned char outputBuffer[BLOWFISH_CHUNK_SIZE + EVP_MAX_BLOCK_LENGTH];
    EVP_CIPHER_CTX ctx;
    unsigned char iv[BF_BLOCK];
    unsigned char key[16];
    int outlen;
};
#endif// SHABACK_BlowfishOutputStream_H
