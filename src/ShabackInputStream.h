#ifndef SHABACK_ShabackInputStream_H
#define SHABACK_ShabackInputStream_H

#include <string>
#include <zlib.h>
#include "lib/File.h"
#include "lib/InputStream.h"
#include "RuntimeConfig.h"

class ShabackInputStream : public InputStream
{
  public:
    ShabackInputStream(RuntimeConfig& config, int compressionAlgorithm, int encryptionAlgorithm);
    ~ShabackInputStream();

    void open(File& file);
    void close();

    int read(char* b, int len);
    int read();

  private:
    int compressionAlgorithm;
    int encryptionAlgorithm;
    File file;
    bool opened;
    InputStream* compressionInputStream;
    InputStream* inputStream;
    InputStream* fileInputStream;
    InputStream* encryptionInputStream;
    RuntimeConfig& config;
};

#endif // SHABACK_ShabackInputStream_H
