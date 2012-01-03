#include "InputStream.h"
#include "Exception.h"

using namespace std;

const long InputStream::SKIP_BUFFER_SIZE = 2048;
char* InputStream::skipBuffer = 0;

int InputStream::read(char* b, int len)
{
  int c = read();
  if (c == -1) {
    return -1;
  }
  b[0] = (char) c;

  int r = 1;
  try {
    for (; r < len; r++) {
      c = read();
      if (c == -1)
        break;
      if (b != 0)
        b[r] = (char) c;
    }
  } catch (IOException ee) {
  }

  return r;
}

bool InputStream::readLine(string& str)
{
  int c;

  c = read();
  if (c < 0)
    return false;

  str.clear();

  for (;;) {
    if (c == '\n') {
      return true;
    } else if (c != '\r') {
      str.append((char*) &c, 1);
    }
    c = read();

    if (c < 0)
      return true;
  }
}

void InputStream::close()
{
}

void InputStream::reset()
{
  throw IOException("reset() not supported");
}

void InputStream::copyTo(OutputStream& destination, int maxBytes)
{
  // TODO: Make sure free() is called!
  char* buffer = (char*) malloc(8192);
  int bytesToRead = maxBytes;

  if (buffer == 0) {
    // TODO: Error handling
    //     throw new MemoryException(JAKELIB_AT2("jakelib.io.InputStream.copyTo"));
  }

  for (;;) {
    int bytesRead =
        read(buffer, maxBytes == -1 ? 8192 : min(8192, bytesToRead));
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
