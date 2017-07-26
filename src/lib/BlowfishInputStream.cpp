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

#include "BlowfishInputStream.h"
#if defined(OPENSSL_FOUND)
#include "Exception.h"
#include "Sha256.h"

using namespace std;

BlowfishInputStream::BlowfishInputStream(string& password, InputStream* in) :
  in(in), finished(false)
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
  EVP_DecryptInit_ex(pctx, EVP_bf_cbc(), NULL, key, iv);

  outlen = 0;
}

BlowfishInputStream::~BlowfishInputStream()
{
  close();
  EVP_CIPHER_CTX_free(pctx);
}

int BlowfishInputStream::read()
{
  throw UnsupportedOperation("BlowfishInputStream::read()");
}

int BlowfishInputStream::read(char* b, int len)
{
  if (len <= 0)
    return -1;

  int bytesRead = in->read((char*) inputBuffer, min(len, BLOWFISH_CHUNK_SIZE));
  if (bytesRead == -1) {
    if (finished)
      return -1;
    finished = true;
    if (!EVP_DecryptFinal(pctx, (unsigned char*) b, &outlen)) {
      throw IOException("EVP_DecryptFinal failed");
    }
    if (outlen == 0) {
      return -1;
    } else {
      return outlen;
    }
  }

  if (!EVP_DecryptUpdate(pctx, (unsigned char*) b, &outlen, (const unsigned char*) inputBuffer, bytesRead)) {
    throw IOException("EVP_DecryptUpdate failed");
  }

  return outlen;
}

void BlowfishInputStream::close()
{
  if (in) {
    in->close();
    in = 0;
  }
}
#endif
