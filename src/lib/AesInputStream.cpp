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

#include "AesInputStream.h"
#if defined(OPENSSL_FOUND)
#include "Exception.h"

using namespace std;

AesInputStream::AesInputStream(unsigned char* key, InputStream* in) :
  in(in), finished(false), key(key)
{
#if defined(HAVE_EVP_CIPHER_CTX_new)
  pctx = EVP_CIPHER_CTX_new();
#else
  EVP_CIPHER_CTX_init(&ctx);
  pctx = &ctx;
#endif

  EVP_EncryptInit_ex(pctx, EVP_aes_256_cbc(), NULL, key, SHABACK_AES_IV);


  int cipherBlockSize = EVP_CIPHER_CTX_block_size(pctx); 
  int cipherKeyLength = EVP_CIPHER_CTX_key_length(pctx);
  int cipherIvLength  = EVP_CIPHER_CTX_iv_length(pctx);

  fprintf(stderr, "INFO(evp_encrypt): Enc Algo:   %s\n", OBJ_nid2ln(EVP_CIPHER_CTX_nid(pctx)));
  fprintf(stderr, "INFO(evp_encrypt): Key:        ");
  for(int i=0; i<cipherKeyLength; i++)
    fprintf(stderr, "%02X", (int)(key[i]));
  fprintf(stderr, "\n");
//   fprintf(stderr, "INFO(evp_encrypt): IV:         ");
//   for(i=0; i<cipherIvLength; i++)
//     fprintf(stderr, "%02X", (int)(iv[i]));
//   fprintf(stderr, "\n");
  fprintf(stderr, "INFO(evp_encrypt): block size: %d\n", cipherBlockSize);
  fprintf(stderr, "INFO(evp_encrypt): key length: %d\n", cipherKeyLength);
  fprintf(stderr, "INFO(evp_encrypt): IV length:  %d\n", cipherIvLength);

  outlen = 0;
}

AesInputStream::~AesInputStream()
{
  close();
  EVP_CIPHER_CTX_free(pctx);
}

int AesInputStream::read()
{
  throw UnsupportedOperation("AesInputStream::read()");
}

int AesInputStream::read(char* b, int len)
{
  if (len <= 0)
    return -1;

  int bytesRead = in->read((char*) inputBuffer, min(len, AES_CHUNK_SIZE));
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

void AesInputStream::close()
{
  if (in) {
    in->close();
    in = 0;
  }
}
#endif
