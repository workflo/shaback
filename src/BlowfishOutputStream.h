#ifndef SHABACK_BlowfishOutputStream_H
#define SHABACK_BlowfishOutputStream_H

#include <string.h>
#include "OutputStream.h"

#define Blowfish_CHUNK_SIZE (16 * 1024)

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
    unsigned char outputBuffer[Blowfish_CHUNK_SIZE];
};
#endif// SHABACK_BlowfishOutputStream_H
