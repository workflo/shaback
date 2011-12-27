#ifndef SHABACK_GzipOutputStream_H
#define SHABACK_GzipOutputStream_H

#include <string.h>
#include <zlib.h>
#include "OutputStream.h"

/**
 * An OutputStream that performs GZip data compression.
 *
 * @class GzipOutputStream
 * @author Florian 'Overflo' Wolff (florian@donuz.de)
 */
class GzipOutputStream : public OutputStream
{
public:
  GzipOutputStream(OutputStream* out); 
  ~GzipOutputStream(); 

  void write(int b);
  void write(const char* b, int len);
  void finish();
  void close();
  
protected:
  OutputStream* out;
  z_stream zipStream;
}; 
#endif// SHABACK_GzipOutputStream_H

