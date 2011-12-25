#include "InputStream.h"
#include "Exception.h"

using namespace std;

const long InputStream::SKIP_BUFFER_SIZE = 2048;
char* InputStream::skipBuffer = 0;


int InputStream::read(char* b, int len)
{
//   if (b == null)
//     throw new NullPointerException();
//   else if (offset < 0 || len < 0)
//     throw new IndexOutOfBoundsException();
//   else if (len == 0)
//     return 0;

  int c = read();
  if (c == -1) {
    return -1;
  }
  b[0] = (char) c;

  int r = 1;
  try {
    for (; r < len ; r++) {
      c = read();
      if (c == -1)
	break;
      if (b != 0)
	b[r] = (char) c;
    }
  } 
  catch (IOException ee) {}

  return r;
}


int InputStream::skip(int n)
{
  // FIXME
  return 0;
}


int InputStream::available()
{
  return 0;
}


void InputStream::close()
{}


void InputStream::mark(int readLimit)
{
  throw IOException("mark() not supported");
}


void InputStream::reset()
{
  throw IOException("reset() not supported");
}


bool InputStream::markSupported()
{
  return false;
}


void InputStream::copyTo(OutputStream& destination, int maxBytes)
{
  char* buffer = (char*) malloc(8192);
  int bytesToRead = maxBytes;

  if (buffer == 0) {
    // TODO: Error handling
//     throw new MemoryException(JAKELIB_AT2("jakelib.io.InputStream.copyTo"));
  }

  for (;;) {
    int bytesRead = read(buffer, maxBytes == -1 ? 8192 : min(8192, bytesToRead));
    if (bytesRead <= 0)
      break;
    destination.write(buffer, bytesRead);
    bytesToRead -= bytesRead;
  }

  free(buffer);
}

void InputStream::copyTo(OutputStream& destination)
{
  copyTo(destination, -1);
}
