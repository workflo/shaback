#ifndef SHABACK_DeflateInputStream_H
#define SHABACK_DeflateInputStream_H

#include <string.h>
#include <zlib.h>
#include "InputStream.h"

#define DEFLATE_CHUNK_SIZE (16 * 1024)

/**
 * An InputStream that performs Deflate data compression.
 *
 * @class DeflateInputStream
 * @author Florian 'Overflo' Wolff (florian@donuz.de)
 */
class DeflateInputStream : public InputStream
{
public:
  DeflateInputStream(InputStream* in); 
  ~DeflateInputStream(); 

  int read();
  int read(char* b, int len);
  void close();

  void setBlocking(bool on) {};
  
protected:
  InputStream* in;
  z_stream zipStream;
  unsigned char readBuffer[DEFLATE_CHUNK_SIZE];
  int ret;
}; 
#endif// SHABACK_DeflateInputStream_H

