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
