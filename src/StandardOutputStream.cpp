#include <fcntl.h>

#include "StandardOutputStream.h"
#include "Exception.h"

using namespace std;


StandardOutputStream::StandardOutputStream(FILE* handle)
  : handle(handle)
{
}

StandardOutputStream::~StandardOutputStream()
{
  close();
}


/*****************************************************************************\
 * write                                                                      |
 *****************************************************************************/
void StandardOutputStream::write(int b)
{
  char c = (char) b;
  write(&c, 1);
}


/*****************************************************************************\
 * write                                                                      |
 *****************************************************************************/
void StandardOutputStream::write(const char* b, int len)
{
  if (::fwrite(b, 1, len, handle) != len) {
    throw Exception::errnoToException();
  }
}


/*****************************************************************************\
 * close                                                                      |
 *****************************************************************************/
void StandardOutputStream::close() 
{
  flush();
}


void StandardOutputStream::flush()
{
  ::fflush(handle);
}

