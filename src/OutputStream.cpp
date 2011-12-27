#include "OutputStream.h"

using namespace std;

OutputStream::~OutputStream() {}


void OutputStream::write(const char* b, int len)
{
//   if (b == 0)
//     throw new NullPointerException();
//   else if (len < 0)
//     throw new IndexOutOfBoundsException();
//   else if (len == 0)
//     return;

  for (int i = 0; i < len ; i++) 
    write(b[i]);
}

