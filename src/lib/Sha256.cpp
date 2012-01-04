#include <iostream>
#include <stdlib.h>
#include "Sha256.h"

using namespace std;


Sha256::Sha256()
{
  ctx = (SHA256_CTX*) malloc(sizeof(SHA256_CTX));
  reset();
}


Sha256::~Sha256()
{
  free(ctx);
  ctx = 0;
}


void Sha256::reset()
{
  SHA256_Init(ctx);
}


void Sha256::update(const void* data, unsigned long len)
{
  SHA256_Update(ctx, data, len);
}

void Sha256::update(std::string& data)
{
  SHA256_Update(ctx, data.data(), data.size());
}


string Sha256::toString()
{
  return hexStr;
}


const unsigned char* Sha256::toBytes()
{
  return bytes;
}


void Sha256::finalize()
{
  char hexChars[SHA256_DIGEST_LENGTH *2 +1];

  SHA256_Final(bytes, ctx);

  for (int x = 0; x < SHA256_DIGEST_LENGTH; x++) {
    hexChars[x*2] = HEX_CHARS[bytes[x] >> 4];
    hexChars[x*2 +1] = HEX_CHARS[bytes[x] & 0x0f];
  }
  hexChars[SHA256_DIGEST_LENGTH *2 ] = 0;

  hexStr = hexChars;
}
