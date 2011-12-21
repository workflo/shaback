#include <iostream>
#include "Exception.h"

using namespace std;


Exception::Exception() {}

Exception::Exception(string msg) : msg(msg) {}

string Exception::getMessage()
{
  return msg;
}

Exception Exception::errnoToException()
{
  int e = errno;
  return errnoToException(e, "");
}

Exception Exception::errnoToException(string filename)
{
  int e = errno;
  return errnoToException(e, filename);
}

Exception Exception::errnoToException(int e, string filename)
{
  string msg;

  switch (e) {
  default:
    char buf[100];
    sprintf(buf, "Unexpected error code: %d", errno);
    return Exception(string(buf));
  }
}


IOException::IOException(string msg) : Exception(msg) {}


FileNotFoundException::FileNotFoundException(string msg, string filename) : IOException(msg), filename(filename) {}

string FileNotFoundException::getFilename()
{
  return filename;
}

