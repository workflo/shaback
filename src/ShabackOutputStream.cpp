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

      //    case ENCRYPTION_AES:
      //      encryptionOutputStream = new AesOutputStream(config.cryptoPassword, outputStream);
      //      outputStream = encryptionOutputStream;
      //      break;

    case ENCRYPTION_NONE:
      break;
  }
#endif

  switch (compressionAlgorithm) {
    case COMPRESSION_DEFLATE:
      compressionOutputStream = new DeflateOutputStream(outputStream);
      outputStream = compressionOutputStream;
      break;

    case COMPRESSION_BZip5:
      compressionOutputStream = new BzOutputStream(outputStream);
      outputStream = compressionOutputStream;
      break;

    case COMPRESSION_BZip1:
      compressionOutputStream = new BzOutputStream(outputStream, 1);
      outputStream = compressionOutputStream;
      break;

    case COMPRESSION_BZip9:
      compressionOutputStream = new BzOutputStream(outputStream, 9);
      outputStream = compressionOutputStream;
      break;

#if defined(LZMA_FOUND)
    case COMPRESSION_LZMA0:
      compressionOutputStream = new LzmaOutputStream(outputStream, 0);
      outputStream = compressionOutputStream;
      break;

    case COMPRESSION_LZMA5:
      compressionOutputStream = new LzmaOutputStream(outputStream, 5);
      outputStream = compressionOutputStream;
      break;

    case COMPRESSION_LZMA9:
      compressionOutputStream = new LzmaOutputStream(outputStream, 9);
      outputStream = compressionOutputStream;
      break;
#endif

    case COMPRESSION_NONE:
      break;
  }
}

void ShabackOutputStream::close()
{
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
