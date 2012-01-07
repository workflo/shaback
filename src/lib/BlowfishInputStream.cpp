#include <iostream>

#include "BlowfishInputStream.h"
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

  EVP_CIPHER_CTX_init(&ctx);
  EVP_DecryptInit_ex(&ctx, EVP_bf_cbc(), NULL, key, iv);

  outlen = 0;
}

BlowfishInputStream::~BlowfishInputStream()
{
  close();
  EVP_CIPHER_CTX_cleanup(&ctx);
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
    if (!EVP_DecryptFinal(&ctx, (unsigned char*) b, &outlen)) {
      throw IOException("EVP_DecryptFinal failed");
    }
    if (outlen == 0) {
      return -1;
    } else {
      return outlen;
    }
  }

  if (!EVP_DecryptUpdate(&ctx, (unsigned char*) b, &outlen, (const unsigned char*) inputBuffer, bytesRead)) {
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
