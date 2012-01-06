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

MissingCryptoPassword::MissingCryptoPassword()
: Exception("Missing crypto password") {}

DeflateException::DeflateException(string msg) : Exception(msg) {}
