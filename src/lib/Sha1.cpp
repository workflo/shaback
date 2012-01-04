#include <iostream>
#include <stdlib.h>
#include "Sha1.h"

using namespace std;


Sha1::Sha1()
{
  ctx = (SHA_CTX*) malloc(sizeof(SHA_CTX));
  reset();
}


Sha1::~Sha1()
{
  free(ctx);
  ctx = 0;
}


void Sha1::reset()
{
  SHA1_Init(ctx);
}


void Sha1::update(const void* data, unsigned long len)
{
  SHA1_Update(ctx, data, len);
}

void Sha1::update(std::string& data)
{
  SHA1_Update(ctx, data.data(), data.size());
}


char Digest::HEX_CHARS[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

string Sha1::toString()
{
  return hexStr;
}


const unsigned char* Sha1::toBytes()
{
  return bytes;
}


void Sha1::finalize()
{
  char hexChars[SHA_DIGEST_LENGTH *2 +1];

  SHA1_Final(bytes, ctx);

  for (int x = 0; x < SHA_DIGEST_LENGTH; x++) {
    hexChars[x*2] = HEX_CHARS[bytes[x] >> 4];
    hexChars[x*2 +1] = HEX_CHARS[bytes[x] & 0x0f];
  }
  hexChars[SHA_DIGEST_LENGTH *2 ] = 0;

  hexStr = hexChars;
}
