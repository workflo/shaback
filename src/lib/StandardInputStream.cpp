#include <fcntl.h>

#include "StandardInputStream.h"
#include "Exception.h"

using namespace std;


StandardInputStream::StandardInputStream(FILE* handle)
  : handle(handle)
{
}

StandardInputStream::~StandardInputStream()
{
  close();
}


/*****************************************************************************\
 * read                                                                       |
 *****************************************************************************/
int StandardInputStream::read()
{
  char b;
  long r;
  r = read(&b, 1);
  if (r == 1)
    return b & 0xff;
  else 
    return -1;
}


/*****************************************************************************\
 * read                                                                       |
 *****************************************************************************/
int StandardInputStream::read(char* b, int len)
{
  int bytesRead = ::fread(b, 1, len, handle);

  if (bytesRead == 0)
    return -1;
  else
    return bytesRead;
}


/*****************************************************************************\
 * close                                                                      |
 *****************************************************************************/
void StandardInputStream::close() {}

