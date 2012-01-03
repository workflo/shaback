#include <iostream>
#include <fcntl.h>

#include "ShabackOutputStream.h"
#include "FileOutputStream.h"
#include "DeflateOutputStream.h"
#include "AesOutputStream.h"
#include "Exception.h"
#include "Repository.h"

using namespace std;

ShabackOutputStream::ShabackOutputStream(RuntimeConfig& config, int compressionAlgorithm, int encryptionAlgorithm) :
  config(config), opened(false), compressionAlgorithm(compressionAlgorithm), encryptionAlgorithm(encryptionAlgorithm)
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
  if (compressionOutputStream)
    delete compressionOutputStream;
  if (encryptionOutputStream)
    delete encryptionOutputStream;
  if (fileOutputStream)
    delete fileOutputStream;
  //   cout << "ShabackOutputStream:~"<<endl;
}

void ShabackOutputStream::open(File& file)
{
  this->file = file;

  string tmpFilename(file.path);
  tmpFilename.append(".tmp");
  this->tmpFile = tmpFilename;

  fileOutputStream = new FileOutputStream(tmpFile);
  outputStream = fileOutputStream;

  switch (encryptionAlgorithm) {
    case ENCRYPTION_AES:
      encryptionOutputStream = new AesOutputStream(config.cryptoPassword, outputStream);
      outputStream = encryptionOutputStream;
      break;

    case ENCRYPTION_NONE:
      break;
  }

  switch (compressionAlgorithm) {
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
  tmpFile.move(file);
}

void ShabackOutputStream::write(string& s)
{
  write(s.data(), s.size());
}

void ShabackOutputStream::write(const char* data, int numBytes)
{
  outputStream->write(data, numBytes);
}
