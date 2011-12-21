#ifndef SHABACK_OutputStream_H
#define SHABACK_OutputStream_H

#include <string>
#include <zlib.h>
#include "File.h"

class OutputStream
{
 public:
  OutputStream(std::string compressionAlgorithm, std::string encryptionAlgorithm);
  ~OutputStream();

  void open(File& file);
  void close();

  void write(std::string& s);
  void write(const char* data, int numBytes);

 private:
  int compressionAlgorithm;
  std::string encryptionAlgorithm;
  File file;
  gzFile gz;
  int fd;
  bool opened;
};

#define COMPRESSION_NONE    0
#define COMPRESSION_GZ      1
#define COMPRESSION_LZO     2

#endif // SHABACK_OutputStream_H
