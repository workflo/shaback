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

#include <iostream>

#include "AesOutputStream.h"
#if defined(OPENSSL_FOUND)
#include "Exception.h"

using namespace std;


AesOutputStream::AesOutputStream(unsigned char* key, OutputStream* out)
  : out(out), key(key)
{
#if defined(HAVE_EVP_CIPHER_CTX_new)
  pctx = EVP_CIPHER_CTX_new();
#else
  EVP_CIPHER_CTX_init(&ctx);
  pctx = &ctx;
#endif

  EVP_EncryptInit_ex(pctx, EVP_aes_256_cbc(), NULL, key, SHABACK_AES_IV);

  outlen = 0;
}

AesOutputStream::~AesOutputStream()
{
  close();
  EVP_CIPHER_CTX_free(pctx);
}


void AesOutputStream::write(int b)
{
  char c = (char) b;
  write(&c, 1);
}


void AesOutputStream::write(const char* b, int len)
{
  while (len > 0) {
    if (!EVP_EncryptUpdate(pctx, outputBuffer, &outlen, (const unsigned char*) b, min(len, AES_CHUNK_SIZE))) {
      throw IOException("EVP_EncryptUpdate failed");
    }

    out->write((const char*) outputBuffer, outlen);

    b += AES_CHUNK_SIZE;
    len -= AES_CHUNK_SIZE;
  }
}


void AesOutputStream::finish()
{
  if (!EVP_CipherFinal_ex(pctx, outputBuffer, &outlen)) {
    throw IOException("EVP_CipherFinal_ex failed");
  }
  out->write((const char*) outputBuffer, outlen);
}


void AesOutputStream::close()
{
  if (out) {
    finish();
    out->close();
    out = 0;
  }
}
#endif
