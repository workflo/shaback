#include <iostream>
#include <fcntl.h>

#include "ShabackInputStream.h"
#include "lib/FileInputStream.h"
#include "lib/DeflateInputStream.h"
#include "lib/BlowfishInputStream.h"
//#include "lib/AesInputStream.h"
#include "lib/Exception.h"
#include "Repository.h"

using namespace std;

ShabackInputStream::ShabackInputStream(RuntimeConfig& config, int compressionAlgorithm, int encryptionAlgorithm) :
  config(config), opened(false), compressionAlgorithm(compressionAlgorithm), encryptionAlgorithm(encryptionAlgorithm)
{
  inputStream = 0;
  compressionInputStream = 0;
  fileInputStream = 0;
  encryptionInputStream = 0;
}

ShabackInputStream::~ShabackInputStream()
{
  close();
  if (compressionInputStream)
    delete compressionInputStream;
  if (encryptionInputStream)
    delete encryptionInputStream;
  if (fileInputStream)
    delete fileInputStream;
}

void ShabackInputStream::open(File& file)
{
  this->file = file;
  fileInputStream = new FileInputStream(file);
  inputStream = fileInputStream;

  switch (encryptionAlgorithm) {
    case ENCRYPTION_BLOWFISH:
      encryptionInputStream = new BlowfishInputStream(config.cryptoPassword, inputStream);
      inputStream = encryptionInputStream;
      break;

//    case ENCRYPTION_AES:
//      encryptionOutputStream = new AesOutputStream(config.cryptoPassword, outputStream);
//      outputStream = encryptionOutputStream;
//      break;

    case ENCRYPTION_NONE:
      break;
  }

  switch (compressionAlgorithm) {
    case COMPRESSION_DEFLATE:
      compressionInputStream = new DeflateInputStream(inputStream);
      inputStream = compressionInputStream;
      break;

    case COMPRESSION_NONE:
      break;
  }

  //   cout << "open: " << file.path << endl;
}

void ShabackInputStream::close()
{
  inputStream->close();
}
