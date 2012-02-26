/*
 * shaback - A hash digest based backup tool.
 * Copyright (C) 2012 Florian Wolff (florian@donuz.de)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <fcntl.h>

#include "ShabackInputStream.h"
//#include "lib/AesInputStream.h"
#include "lib/BlowfishInputStream.h"
#include "lib/BzInputStream.h"
#include "lib/DeflateInputStream.h"
#include "lib/Exception.h"
#include "lib/FileInputStream.h"
#include "lib/LzmaInputStream.h"
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

    case COMPRESSION_BZip5:
    case COMPRESSION_BZip1:
    case COMPRESSION_BZip9:
      compressionInputStream = new BzInputStream(inputStream);
      inputStream = compressionInputStream;
      break;

    case COMPRESSION_LZMA0:
    case COMPRESSION_LZMA5:
    case COMPRESSION_LZMA9:
      compressionInputStream = new LzmaInputStream(inputStream);
      inputStream = compressionInputStream;
      break;

    case COMPRESSION_NONE:
      break;
  }
}

void ShabackInputStream::close()
{
  if (inputStream)
    inputStream->close();
}

int ShabackInputStream::read()
{
  throw UnsupportedOperation("ShabackInputStream::read()");
}

int ShabackInputStream::read(char* b, int len)
{
  return inputStream->read(b, len);
}
