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

#include "ShabackOutputStream.h"
#include "lib/AesOutputStream.h"
#include "lib/BlowfishOutputStream.h"
#include "lib/BzOutputStream.h"
#include "lib/DeflateOutputStream.h"
#include "lib/Exception.h"
#include "lib/FileOutputStream.h"
#include "lib/LzmaOutputStream.h"
#include "lib/ZStdOutputStream.h"
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

  if (compressionOutputStream)
    delete compressionOutputStream;
  if (encryptionOutputStream)
    delete encryptionOutputStream;
  if (fileOutputStream)
    delete fileOutputStream;
}


OutputStream* ShabackOutputStream::createCompressionStream(OutputStream* outputStream, int compressionAlgorithm)
{
  switch (compressionAlgorithm) {
    case COMPRESSION_DEFLATE:
      return new DeflateOutputStream(outputStream);

    case COMPRESSION_BZip5:
      return new BzOutputStream(outputStream);

    case COMPRESSION_BZip1:
      return new BzOutputStream(outputStream, 1);

    case COMPRESSION_BZip9:
      return new BzOutputStream(outputStream, 9);

#if defined(ZSTD_FOUND)
    case COMPRESSION_ZSTD1:
      return new ZStdOutputStream(outputStream, 0);

    case COMPRESSION_ZSTD5:
      return new ZStdOutputStream(outputStream, 5);

    case COMPRESSION_ZSTD9:
      return new ZStdOutputStream(outputStream, 9);
#endif

#if defined(LZMA_FOUND)
    case COMPRESSION_LZMA0:
      return new LzmaOutputStream(outputStream, 0);

    case COMPRESSION_LZMA5:
      return new LzmaOutputStream(outputStream, 5);

    case COMPRESSION_LZMA9:
      return new LzmaOutputStream(outputStream, 9);
#endif

    case COMPRESSION_NONE:
      return 0;

    default:
      throw Exception("Unexpected compression algorithm");
  }
}


void ShabackOutputStream::open(File& file)
{
  this->file = file;

  string tmpFilename(file.path);
  tmpFilename.append(".tmp");
  this->tmpFile = tmpFilename;

  fileOutputStream = new FileOutputStream(tmpFile);
  outputStream = fileOutputStream;

#if defined(OPENSSL_FOUND)
  switch (encryptionAlgorithm) {
    case ENCRYPTION_BLOWFISH:
      encryptionOutputStream = new BlowfishOutputStream(config.cryptoPassword, outputStream);
      outputStream = encryptionOutputStream;
      break;

    case ENCRYPTION_NONE:
      break;
  }
#endif

  compressionOutputStream = createCompressionStream(outputStream, compressionAlgorithm);
  if (compressionOutputStream) {
    outputStream = compressionOutputStream;
  } 
}

void ShabackOutputStream::close()
{
  if (compressionOutputStream) {
    compressionOutputStream->close();
    compressionOutputStream = 0;
  }
  if (outputStream) {
    outputStream->close();
    outputStream = 0;
  }
}

void ShabackOutputStream::finish()
{
  close();
  tmpFile.move(file);
}

void ShabackOutputStream::write(string& s)
{
  write(s.data(), s.size());
}

void ShabackOutputStream::write(const char* s)
{
  write(s, strlen(s));
}

void ShabackOutputStream::write(const char* data, int numBytes)
{
  outputStream->write(data, numBytes);
}
