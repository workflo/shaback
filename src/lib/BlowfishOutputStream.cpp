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

#include "BlowfishOutputStream.h"
#if defined(OPENSSL_FOUND)
#include "Exception.h"
#include "Sha256.h"

using namespace std;

BlowfishOutputStream::BlowfishOutputStream(string& password, OutputStream* out) :
  out(out)
{
  // Static Initialization Vector:
  strncpy((char*) iv, SHABACK_IV, BF_BLOCK);

  // Compute SHA-256 digest of password:
  Sha256 sha;
  sha.update(password);
  sha.finalize();

  // Use first 16 bytes of digest as key:
  strncpy((char*) key, (const char*) sha.toBytes(), 16);

#if defined(HAVE_EVP_CIPHER_CTX_new)
  pctx = EVP_CIPHER_CTX_new();
#else
  EVP_CIPHER_CTX_init(&ctx);
  pctx = &ctx;
#endif

  EVP_EncryptInit_ex(pctx, EVP_bf_cbc(), NULL, key, iv);

  outlen = 0;
}

BlowfishOutputStream::~BlowfishOutputStream()
{
  close();
  EVP_CIPHER_CTX_free(pctx);
}

void BlowfishOutputStream::write(int b)
{
  char c = (char) b;
  write(&c, 1);
}

void BlowfishOutputStream::write(const char* b, int len)
{
  while (len > 0) {
    if (!EVP_EncryptUpdate(pctx, outputBuffer, &outlen, (const unsigned char*) b, min(len, BLOWFISH_CHUNK_SIZE))) {
      throw IOException("EVP_EncryptUpdate failed");
    }

    out->write((const char*) outputBuffer, outlen);

    b += BLOWFISH_CHUNK_SIZE;
    len -= BLOWFISH_CHUNK_SIZE;
  }
}

void BlowfishOutputStream::finish()
{
  if (!EVP_CipherFinal_ex(pctx, outputBuffer, &outlen)) {
    throw IOException("EVP_CipherFinal_ex failed");
  }
  out->write((const char*) outputBuffer, outlen);
}

void BlowfishOutputStream::close()
{
  if (out) {
    finish();
    out->close();
    out = 0;
  }
}
#endif
