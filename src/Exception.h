#ifndef Exception_H
#define Exception_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <exception>
#include <errno.h>


class Exception
{
 public:
  Exception();
  Exception(std::string msg);
  virtual std::string getMessage();
  static Exception errnoToException();
  static Exception errnoToException(std::string filename);
  static Exception errnoToException(int e, std::string filename);

 protected:
  std::string msg;
};

class IOException : public Exception 
{
 public:
  IOException(std::string msg);
};

class FileNotFoundException : public IOException 
{
 public:
  FileNotFoundException(std::string filename);
  virtual std::string getFilename();

 protected:
  std::string filename;
};

class UnsupportedCompressionAlgorithm : public Exception 
{
 public:
  UnsupportedCompressionAlgorithm(std::string algo);
};

class UnsupportedEncryptionAlgorithm : public Exception 
{
 public:
  UnsupportedEncryptionAlgorithm(std::string algo);
};


#endif // Exception_H
