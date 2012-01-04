#ifndef SHABACK_Sha1_H
#define SHABACK_Sha1_H

#include <string>
#include <openssl/sha.h>
#include "Digest.h"

class Sha1: public Digest
{
  public:
    Sha1();
    ~Sha1();

    void reset();

    void update(const void* data, unsigned long len);
    void update(std::string& data);

    std::string toString();
    const unsigned char* toBytes();
    void finalize();

  private:
    std::string hexStr;
    SHA_CTX *ctx;
    unsigned char bytes[SHA_DIGEST_LENGTH];
};

#endif // SHABACK_Sha1_H
