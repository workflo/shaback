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
#if !defined(SHABACK_AesOutputStream_H) && defined(OPENSSL_FOUND)
#define SHABACK_AesOutputStream_H

#include <string.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include "OutputStream.h"

// AES 256 CBC:
// block size: 16
// key length: 32
// IV length:  16

#define SHABACK_AES_IV ((unsigned char*) "SHABACK2_h29skHs")
#define AES_CHUNK_SIZE (AES_BLOCK_SIZE * 1024 / 8)

class AesOutputStream: public OutputStream
{
  public:
    AesOutputStream(unsigned char* key, OutputStream* out);
    ~AesOutputStream();

    void write(int b);
    void write(const char* b, int len);
    void finish();
    void close();

  protected:
    unsigned char* key;
    OutputStream* out;
    unsigned char outputBuffer[AES_CHUNK_SIZE + EVP_MAX_BLOCK_LENGTH];
#if defined(HAVE_EVP_CIPHER_CTX_new)
    EVP_CIPHER_CTX *pctx;
#else
    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX *pctx;
#endif
    int outlen;
};
#endif// SHABACK_AesOutputStream_H
