#ifndef SHABACK_StandardOutputStream_H
#define SHABACK_StandardOutputStream_H

#include <string.h>
#include "File.h"
#include "OutputStream.h"

/**
 * @class StandardOutputStream
 * @author Florian 'Overflo' Wolff (florian@donuz.de)
 */
class StandardOutputStream : public OutputStream
{
public:
  StandardOutputStream(FILE* handle);
  ~StandardOutputStream(); 

  void write(int b);
  void write(const char* b, int len);
  void close();
  void flush();
  
protected:
  FILE* handle;
};
#endif // SHABACK_StandardOutputStream_H
