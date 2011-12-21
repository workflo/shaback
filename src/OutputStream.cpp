#include <iostream>
//#include <stdlib.h>/
//#include <stdio.h>
#include <fcntl.h>

#include "OutputStream.h"
#include "Exception.h"

using namespace std;


OutputStream::OutputStream(string compressionAlgorithm, string encryptionAlgorithm)
  : opened(false)
{
  if (compressionAlgorithm == "GZ") {
    this->compressionAlgorithm = COMPRESSION_GZ;
//   } else if (compressionAlgorithm == "LZO") {
//     this->compressionAlgorithm = COMPRESSION_LZO;
  } else if (compressionAlgorithm.empty()) {
    this->compressionAlgorithm = COMPRESSION_NONE;
  } else {
    throw UnsupportedCompressionAlgorithm(compressionAlgorithm);
  }
}

OutputStream::~OutputStream()
{
  close();
}


void OutputStream::open(File& file)
{
  this->file = file;

  switch(compressionAlgorithm) {
    
  case COMPRESSION_GZ:
    gz = ::gzopen(file.path.c_str(), "wb");
    if (gz == 0) {
      throw Exception::errnoToException(file.path);
    }
    opened = true;
    break;

  case COMPRESSION_NONE:
    fd = ::open(file.path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0777);
    if (fd == 0) {
      throw Exception::errnoToException(file.path);
    }
    opened = true;
    break;
    
  }
//   cout << "open: " << file.path << endl;
}


void OutputStream::close()
{
  if (opened) {
    switch(compressionAlgorithm) {
    
    case COMPRESSION_GZ:
      gzclose(gz);
      break;

    case COMPRESSION_NONE:
      ::close(fd);
      break;
    }
  
    opened = false;

///     cout << "close: " << file.path << endl;
  }
}


void OutputStream::write(string& s)
{
  write(s.data(), s.size());
}


void OutputStream::write(const char* data, int numBytes)
{
    switch(compressionAlgorithm) {
    
    case COMPRESSION_GZ:
      if (gzwrite(gz, data, numBytes) != numBytes) {
	throw Exception::errnoToException(file.path);
      }
      break;

    case COMPRESSION_NONE:
      if (::write(fd, data, numBytes) != numBytes) {
	throw Exception::errnoToException(file.path);
      }
      break;
    }
}

////
