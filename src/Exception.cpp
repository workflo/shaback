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
    if (filename.empty()) {
      return Exception(string(strerror(e)));
    } else {
      return Exception(string(strerror(e)).append(": ").append(filename));
    }
  }
}


IOException::IOException(string msg) : Exception(msg) {}


FileNotFoundException::FileNotFoundException(string filename) 
  : IOException(string("File not found: ").append(filename)),
    filename(filename) {}

string FileNotFoundException::getFilename()
{
  return filename;
}

