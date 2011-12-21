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
  FileNotFoundException(std::string msg, std::string filename);
  virtual std::string getFilename();

 protected:
  std::string filename;
};


#endif // Exception_H
