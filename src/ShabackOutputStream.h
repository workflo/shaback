#ifndef SHABACK_ShabackOutputStream_H
#define SHABACK_ShabackOutputStream_H

#include <string>
#include <zlib.h>
#include "File.h"
#include "OutputStream.h"

class ShabackOutputStream
{
 public:
  ShabackOutputStream(int compressionAlgorithm, int encryptionAlgorithm);
  ~ShabackOutputStream();

  void open(File& file);
  void close();

  void write(std::string& s);
  void write(const char* data, int numBytes);

 private:
  int compressionAlgorithm;
  int encryptionAlgorithm;
  File file;
  File tmpFile;
  gzFile gz;
  int fd;
  bool opened;
  OutputStream* compressionOutputStream;
  OutputStream* outputStream;
  OutputStream* fileOutputStream;
  OutputStream* encryptionOutputStream;
};

#endif // SHABACK_ShabackOutputStream_H
