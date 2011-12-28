#include <iostream>
#include <fcntl.h>

#include "ShabackOutputStream.h"
#include "FileOutputStream.h"
#include "DeflateOutputStream.h"
#include "Exception.h"
#include "Repository.h"

using namespace std;


ShabackOutputStream::ShabackOutputStream(int compressionAlgorithm, int encryptionAlgorithm)
  : opened(false), compressionAlgorithm(compressionAlgorithm),
    encryptionAlgorithm(encryptionAlgorithm)
{
  outputStream = 0;
  compressionOutputStream = 0;
  fileOutputStream = 0;
  encryptionOutputStream = 0;
}

ShabackOutputStream::~ShabackOutputStream()
{
  close();
//   cout << "ShabackOutputStream:~"<<endl;
  if (compressionOutputStream) delete compressionOutputStream;
  if (encryptionOutputStream) delete encryptionOutputStream;
  if (fileOutputStream) delete fileOutputStream;
//   cout << "ShabackOutputStream:~"<<endl;
}


void ShabackOutputStream::open(File& file)
{
  this->file = file;
  fileOutputStream = new FileOutputStream(file);
  outputStream = fileOutputStream;

  switch(compressionAlgorithm) {
    
  case COMPRESSION_DEFLATE:
    compressionOutputStream = new DeflateOutputStream(outputStream);
    outputStream = compressionOutputStream;
    break;

  case COMPRESSION_NONE:
    break;
    
  }
//   cout << "open: " << file.path << endl;
}


void ShabackOutputStream::close()
{
  outputStream->close();
}


void ShabackOutputStream::write(string& s)
{
  write(s.data(), s.size());
}


void ShabackOutputStream::write(const char* data, int numBytes)
{
  outputStream->write(data, numBytes);
}
