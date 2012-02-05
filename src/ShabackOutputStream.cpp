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
#include "lib/FileOutputStream.h"
#include "lib/DeflateOutputStream.h"
#include "lib/BlowfishOutputStream.h"
#include "lib/AesOutputStream.h"
#include "lib/Exception.h"
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

  switch (compressionAlgorithm) {
    case COMPRESSION_DEFLATE:
      compressionOutputStream = new DeflateOutputStream(outputStream);
      outputStream = compressionOutputStream;
      break;

    case COMPRESSION_NONE:
      break;
  }
}

void ShabackOutputStream::close()
{
  if (outputStream)
    outputStream->close();
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
