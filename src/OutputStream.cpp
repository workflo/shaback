#include "OutputStream.h"

using namespace std;

void OutputStream::write(const char* b, int offset, int len)
{
//   if (b == 0)
//     throw new NullPointerException();
//   else if (len < 0)
//     throw new IndexOutOfBoundsException();
//   else if (len == 0)
//     return;

  for (int i = 0; i < len ; i++) 
    write(b[offset + i]);
}

