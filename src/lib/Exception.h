#ifndef SHABACK_Exception_H
#define SHABACK_Exception_H

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

class IOException: public Exception
{
  public:
    IOException(std::string msg);
};

class FileNotFoundException: public IOException
{
  public:
    FileNotFoundException(std::string filename);
    virtual std::string getFilename();

  protected:
    std::string filename;
};

class IllegalStateException : public Exception
{
  public:
    IllegalStateException(std::string msg);
};

class UnsupportedCompressionAlgorithm: public Exception
{
  public:
    UnsupportedCompressionAlgorithm(std::string algo);
};

class UnsupportedEncryptionAlgorithm: public Exception
{
  public:
    UnsupportedEncryptionAlgorithm(std::string algo);
};

class UnsupportedOperation: public Exception
{
  public:
    UnsupportedOperation(std::string op);
};

class MissingCryptoPassword: public Exception
{
  public:
    MissingCryptoPassword();
};

class DeflateException : public Exception
{
  public:
    DeflateException(std::string msg);
};

#endif // SHABACK_Exception_H
