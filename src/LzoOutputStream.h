#ifndef SHABACK_LzoOutputStream_H
#define SHABACK_LzoOutputStream_H

#include <string.h>
#include <lzo/lzo1x.h>
#include "OutputStream.h"

/**
 * An OutputStream that performs LZO data compression.
 *
 * @class LzoOutputStream
 * @author Florian 'Overflo' Wolff (florian@donuz.de)
 */
class LzoOutputStream : public OutputStream
{
public:
  LzoOutputStream(OutputStream* out); 
  ~LzoOutputStream(); 

  void write(int b);
  void write(const char* b, int len);
  void finish();
  void close();
  
protected:
  OutputStream* out;
  unsigned char* workingMemory;
  unsigned char* outputData;
}; 
#endif// SHABACK_LzoOutputStream_H

