#ifndef SHABACK_ShabackException_H
#define SHABACK_ShabackException_H

#include "lib/Exception.h"

class RestoreException : public Exception
{
  public:
    RestoreException(std::string msg);
};

class InvalidTreeFile : public Exception
{
  public:
    InvalidTreeFile(std::string nsg);
};
#endif // SHABACK_ShabackException_H
