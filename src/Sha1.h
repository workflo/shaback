#ifndef Sha1_H
#define Sha1_H

#include <string>
#include <openssl/sha.h>

class Sha1
{
public:
    Sha1();
    ~Sha1();

    void reset();

    void update(const void* data, unsigned long len);
    void update(std::string& data);

    std::string toString();
    void finalize();
 private:
    std::string hexStr;
    SHA_CTX *ctx;
};

#endif // Sha1_H


