#ifndef SHABACK_AesOutputStream_H
#define SHABACK_AesOutputStream_H

#include <string.h>
#include "OutputStream.h"

#define AES_CHUNK_SIZE (16 * 1024)

class AesOutputStream: public OutputStream
{
  public:
    AesOutputStream(std::string& password, OutputStream* out);
    ~AesOutputStream();

    void write(int b);
    void write(const char* b, int len);
    void finish();
    void close();

  protected:
    OutputStream* out;
    unsigned char outputBuffer[AES_CHUNK_SIZE];
};
#endif// SHABACK_AesOutputStream_H
