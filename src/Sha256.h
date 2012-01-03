#ifndef SHABACK_Sha256_H
#define SHABACK_Sha256_H

#include <string>
#include <openssl/sha.h>
#include "Digest.h"

class Sha256: public Digest
{
  public:
    Sha256();
    ~Sha256();

    void reset();

    void update(const void* data, unsigned long len);
    void update(std::string& data);

    std::string toString();
    const unsigned char* toBytes();
    void finalize();

  private:
    std::string hexStr;
    SHA256_CTX *ctx;
    unsigned char bytes[SHA256_DIGEST_LENGTH];
};

#endif // SHABACK_Sha256_H
