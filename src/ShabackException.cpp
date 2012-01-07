#include <iostream>
#include <string.h>

#include "ShabackException.h"

using namespace std;

RestoreException::RestoreException(string msg) :
  Exception(msg)
{
}

InvalidTreeFile::InvalidTreeFile(string msg) :
  Exception(msg)
{
}

GarbageCollectionException::GarbageCollectionException(string msg) :
  Exception(msg)
{
}
