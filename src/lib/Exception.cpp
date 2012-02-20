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
#include <string.h>
#ifdef WIN32
# include <windows.h>
#endif

#include "Exception.h"

using namespace std;

Exception::Exception()
{
}

Exception::Exception(string msg) :
  msg(msg)
{
}

string Exception::getMessage()
{
  return msg;
}

Exception Exception::errnoToException()
{
#ifdef WIN32
  return errnoToException(GetLastError(), "");
#else
  int e = errno;
  return errnoToException(e, "");
#endif
}

Exception Exception::errnoToException(string filename)
{
#ifdef WIN32
  return errnoToException(GetLastError(), filename);
#else
  int e = errno;
  return errnoToException(e, filename);
#endif
}

Exception Exception::errnoToException(int e, string filename)
{
  string msg;

  switch (e) {
    case ENOENT:
      return FileNotFoundException(filename);

    default:
      if (filename.empty()) {
        return IOException(string(strerror(e)));
      } else {
        return IOException(string(strerror(e)).append(": ").append(filename));
      }
  }
}

IOException::IOException(string msg) :
  Exception(msg)
{
}

FileNotFoundException::FileNotFoundException(string filename) :
  IOException(string("File not found: ").append(filename)), filename(filename)
{
}

string FileNotFoundException::getFilename()
{
  return filename;
}

IllegalStateException::IllegalStateException(string msg) :
  Exception(msg)
{
}

UnsupportedCompressionAlgorithm::UnsupportedCompressionAlgorithm(string algo) :
  Exception(string("Unsupported compression algorithm: ").append(algo))
{
}

UnsupportedEncryptionAlgorithm::UnsupportedEncryptionAlgorithm(string algo) :
  Exception(string("Unsupported encryption algorithm: ").append(algo))
{
}

UnsupportedOperation::UnsupportedOperation(string op) :
  Exception(string("Unsupported operation: ").append(op))
{
}

MissingCryptoPassword::MissingCryptoPassword() :
  Exception("Missing crypto password")
{
}

DeflateException::DeflateException(string msg) :
  Exception(msg)
{
}

BzException::BzException(string msg) :
  Exception(msg)
{
}
