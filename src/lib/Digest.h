#ifndef SHABACK_Digest_H
#define SHABACK_Digest_H

#include <string>
#include <openssl/sha.h>

class Digest
{
  public:
    virtual void reset() = 0;

    virtual void update(const void* data, unsigned long len) = 0;
    virtual void update(std::string& data) = 0;

    virtual std::string toString() = 0;
    virtual const unsigned char* toBytes() = 0;
    virtual void finalize() = 0;

    static bool looksLikeDigest(std::string& str);

  protected:
    std::string hexStr;

    static char HEX_CHARS[];
};

#endif // SHABACK_Digest_H
