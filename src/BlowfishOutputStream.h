#ifndef SHABACK_BlowfishOutputStream_H
#define SHABACK_BlowfishOutputStream_H

#include <string.h>
#include <openssl/evp.h>
#include <openssl/blowfish.h>
#include "OutputStream.h"

#define SHABACK_IV "SHABACK2"
#define BLOWFISH_CHUNK_SIZE (BF_BLOCK * 1024)

class BlowfishOutputStream: public OutputStream
{
  public:
    BlowfishOutputStream(std::string& password, OutputStream* out);
    ~BlowfishOutputStream();

    void write(int b);
    void write(const char* b, int len);
    void finish();
    void close();

  protected:
    OutputStream* out;
    unsigned char outputBuffer[BLOWFISH_CHUNK_SIZE + EVP_MAX_BLOCK_LENGTH];
    EVP_CIPHER_CTX ctx;
    unsigned char iv[BF_BLOCK];
    unsigned char key[16];
    int outlen;
//    unsigned char remainder[EVP_MAX_BLOCK_LENGTH];
//    int remainderLen;
};
#endif// SHABACK_BlowfishOutputStream_H
