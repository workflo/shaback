#include <iostream>

#include "DeflateInputStream.h"
#include "Exception.h"

using namespace std;


DeflateInputStream::DeflateInputStream(InputStream* in)
  : in(in)
{
  ret = Z_OK;
  zipStream.zalloc = Z_NULL;
  zipStream.zfree = Z_NULL;
  zipStream.opaque = Z_NULL;

  if (inflateInit(&zipStream) != Z_OK) {
    // TODO: Throw exception
    cerr << "DeflateInputStream error" << endl;
  }
}

DeflateInputStream::~DeflateInputStream()
{
  close();
}


int DeflateInputStream::read()
{
  throw UnsupportedOperation("read()");
}


int DeflateInputStream::read(char* b, int len)
{
  if (zipStream.avail_in == 0) {
    int bytesRead = in->read((char*) readBuffer, min(len, DEFLATE_CHUNK_SIZE));

    zipStream.avail_in = bytesRead;
    zipStream.next_in = readBuffer;

    if (bytesRead == -1) return -1;
  }
    
  zipStream.avail_out = len;
  zipStream.next_out = (unsigned char*) b;

  ret = inflate(&zipStream, Z_NO_FLUSH);

  if (ret == Z_STREAM_END && zipStream.avail_out == len) {
    return -1;
  } else if (ret == Z_STREAM_ERROR) {
    // TODO: Throw exception
  }

  return DEFLATE_CHUNK_SIZE - zipStream.avail_out;
}


void DeflateInputStream::close()
{
  if (ret != Z_STREAM_END) {
    inflateEnd(&zipStream);
    in->close();
    ret = Z_STREAM_END;
  }
}

