#include <iostream>

#include "AesOutputStream.h"
#include "Exception.h"

using namespace std;


AesOutputStream::AesOutputStream(string& password, OutputStream* out)
  : out(out)
{
}

AesOutputStream::~AesOutputStream()
{
  close();
}


void AesOutputStream::write(int b)
{
  char c = (char) b;
  write(&c, 1);
}


void AesOutputStream::write(const char* b, int len)
{
  if (len <= 0) return;
  out->write(b, len);
}


void AesOutputStream::finish()
{
}


void AesOutputStream::close()
{
  if (out) {
    finish();
    out->close();
    out = 0;
  }
}

