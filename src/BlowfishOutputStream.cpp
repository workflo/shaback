#include <iostream>

#include "BlowfishOutputStream.h"
#include "Exception.h"

using namespace std;


BlowfishOutputStream::BlowfishOutputStream(string& password, OutputStream* out)
  : out(out)
{
}

BlowfishOutputStream::~BlowfishOutputStream()
{
  close();
}


void BlowfishOutputStream::write(int b)
{
  char c = (char) b;
  write(&c, 1);
}


void BlowfishOutputStream::write(const char* b, int len)
{
  if (len <= 0) return;
  out->write(b, len);
}


void BlowfishOutputStream::finish()
{
}


void BlowfishOutputStream::close()
{
  if (out) {
    finish();
    out->close();
    out = 0;
  }
}

