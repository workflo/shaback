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
#include <bzlib.h>
#if defined(LZMA_FOUND)
# include <lzma.h>
#endif
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

UnsupportedRepositoryFormat::UnsupportedRepositoryFormat(string fmt) :
  Exception(string("Unsupported repository format: ").append(fmt))
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

BzException::BzException(string msg, int err)
{
  switch (err) {
    case BZ_PARAM_ERROR:
      this->msg = msg.append(": BZ_PARAM_ERROR");
      break;

    case BZ_MEM_ERROR:
      this->msg = msg.append(": BZ_MEM_ERROR");
      break;

    case BZ_DATA_ERROR:
      this->msg = msg.append(": BZ_DATA_ERROR");
      break;

    case BZ_IO_ERROR:
      this->msg = msg.append(": BZ_IO_ERROR");
      break;

    default: {
      char buf[10];
      sprintf(buf, "%i", err);
      this->msg = msg.append(": error code=").append(buf);
    }
  }
}

LzmaException::LzmaException(string msg) :
  Exception(msg)
{
}

LzmaException::LzmaException(string msg, int err)
{
  switch (err) {
#if defined(LZMA_FOUND)
    case LZMA_OPTIONS_ERROR:
      this->msg = msg.append(": LZMA_OPTIONS_ERROR");
      break;

    case LZMA_MEM_ERROR:
      this->msg = msg.append(": LZMA_MEM_ERROR");
      break;

    case LZMA_DATA_ERROR:
      this->msg = msg.append(": LZMA_DATA_ERROR");
      break;
#endif

    default: {
      char buf[10];
      sprintf(buf, "%i", err);
      this->msg = msg.append(": error code=").append(buf);
    }
  }
}

