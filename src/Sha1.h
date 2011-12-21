#ifndef SHABACK_Sha1_H
#define SHABACK_Sha1_H

#include <string>
#include <openssl/sha.h>
#include "Digest.h"

class Sha1 : public Digest
{
 public:
  Sha1();
  ~Sha1();

  void reset();

  void update(const void* data, unsigned long len);
  void update(std::string& data);

  std::string toString();
  void finalize();

 protected:
  static char HEX_CHARS[];

 private:
  std::string hexStr;
  SHA_CTX *ctx;
};

#endif // SHABACK_Sha1_H
