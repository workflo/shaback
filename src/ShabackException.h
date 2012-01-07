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
    InvalidTreeFile(std::string msg);
};

class GarbageCollectionException : public Exception
{
  public:
    GarbageCollectionException(std::string msg);
};
#endif // SHABACK_ShabackException_H
