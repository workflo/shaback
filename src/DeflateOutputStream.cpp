#include <iostream>

#include "DeflateOutputStream.h"
#include "Exception.h"

using namespace std;


DeflateOutputStream::DeflateOutputStream(OutputStream* out)
  : out(out)
{
  ret = 0;
  zipStream.zalloc = Z_NULL;
  zipStream.zfree = Z_NULL;
  zipStream.opaque = Z_NULL;
  if (deflateInit(&zipStream, Z_DEFAULT_COMPRESSION) != Z_OK) {
    // TODO: Throw exception
    cout << "DeflateOutputStream error" << endl;
  }
}

DeflateOutputStream::~DeflateOutputStream()
{
  close();
}


void DeflateOutputStream::write(int b)
{
  char c = (char) b;
  write(&c, 1);
}


void DeflateOutputStream::write(const char* b, int len)
{
  if (len <= 0) return;

  zipStream.avail_in = len;
  zipStream.next_in = (unsigned char*) b;
    
  do {
    zipStream.avail_out = DEFLATE_CHUNK_SIZE;
    zipStream.next_out = outputBuffer;
      
    ret = deflate(&zipStream, Z_NO_FLUSH);
    if (ret == Z_STREAM_ERROR) {
      // TODO: Throw exception
    }
    out->write((const char*) outputBuffer, DEFLATE_CHUNK_SIZE - zipStream.avail_out);
  } while (zipStream.avail_out == 0);
}


void DeflateOutputStream::finish()
{
  unsigned char inBuf[10];

  zipStream.avail_in = 0;
  zipStream.next_in = inBuf;
    
  do {
    zipStream.avail_out = DEFLATE_CHUNK_SIZE;
    zipStream.next_out = outputBuffer;
     
    ret = deflate(&zipStream, Z_FINISH);
    if (ret == Z_STREAM_ERROR) {
      // TODO: Throw exception
    }
    out->write((const char*) outputBuffer, DEFLATE_CHUNK_SIZE - zipStream.avail_out);
  } while (zipStream.avail_out == 0);

  deflateEnd(&zipStream);
}


void DeflateOutputStream::close()
{
  if (ret != Z_STREAM_END) {
    finish();
    out->close();
  }
}

