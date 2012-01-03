#include <iostream>

#include "BlowfishInputStream.h"
#include "Exception.h"
#include "Sha256.h"

using namespace std;

BlowfishInputStream::BlowfishInputStream(string& password, InputStream* in) :
  in(in)
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
  throw UnsupportedOperation("read()");
}

int BlowfishInputStream::read(char* b, int len)
{
  if (len <= 0)
    return -1;

  int bytesRead = in->read((char*) inputBuffer, min(len, BLOWFISH_CHUNK_SIZE));
  if (bytesRead == -1) return -1;

  if (!EVP_DecryptUpdate(&ctx, (unsigned char*) b, &outlen, (const unsigned char*) inputBuffer, len)) {
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
