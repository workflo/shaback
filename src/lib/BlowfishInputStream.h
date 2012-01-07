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

#ifndef SHABACK_BlowfishInputStream_H
#define SHABACK_BlowfishInputStream_H

#include <string.h>
#include <openssl/evp.h>
#include <openssl/blowfish.h>
#include "InputStream.h"
#include "BlowfishOutputStream.h"


class BlowfishInputStream: public InputStream
{
  public:
    BlowfishInputStream(std::string& password, InputStream* in);
    ~BlowfishInputStream();

    int read();
    int read(char* b, int len);
    void close();

  protected:
    InputStream* in;
    unsigned char inputBuffer[BLOWFISH_CHUNK_SIZE + EVP_MAX_BLOCK_LENGTH];
    EVP_CIPHER_CTX ctx;
    unsigned char iv[BF_BLOCK];
    unsigned char key[16];
    int outlen;
    bool finished;
};
#endif// SHABACK_BlowfishInputStream_H
