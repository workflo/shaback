#ifndef SHABACK_OutputStream_H
#define SHABACK_OutputStream_H

#include <string>
#include <zlib.h>
#include "File.h"

class OutputStream
{
 public:
  OutputStream(int compressionAlgorithm, int encryptionAlgorithm);
  ~OutputStream();

  void open(File& file);
  void close();

  void write(std::string& s);
  void write(const char* data, int numBytes);

 private:
  int compressionAlgorithm;
  int encryptionAlgorithm;
  File file;
  gzFile gz;
  int fd;
  bool opened;
};

#endif // SHABACK_OutputStream_H
