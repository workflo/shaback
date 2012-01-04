#include <iostream>
#include <string.h>

#include "ShabackException.h"

using namespace std;

RestoreException::RestoreException(string msg) :
  Exception(msg)
{
}
