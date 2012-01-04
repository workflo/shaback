#ifndef SHABACK_DeflateOutputStream_H
#define SHABACK_DeflateOutputStream_H

#include <string.h>
#include <zlib.h>
#include "OutputStream.h"

#define DEFLATE_CHUNK_SIZE (16 * 1024)

/**
 * An OutputStream that performs Deflate data compression.
 *
 * @class DeflateOutputStream
 * @author Florian 'Overflo' Wolff (florian@donuz.de)
 */
class DeflateOutputStream : public OutputStream
{
public:
  DeflateOutputStream(OutputStream* out); 
  ~DeflateOutputStream(); 

  void write(int b);
  void write(const char* b, int len);
  void finish();
  void close();
  
protected:
  OutputStream* out;
  z_stream zipStream;
  unsigned char outputBuffer[DEFLATE_CHUNK_SIZE];
  int ret;
}; 
#endif// SHABACK_DeflateOutputStream_H

