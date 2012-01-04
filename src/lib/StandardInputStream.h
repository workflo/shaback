#ifndef SHABACK_StandardInputStream_H
#define SHABACK_StandardInputStream_H

#include <string.h>
#include "File.h"
#include "InputStream.h"

/**
 * @class StandardInputStream
 * @author Florian 'Overflo' Wolff (florian@donuz.de)
 */
class StandardInputStream : public InputStream
{
public:
  StandardInputStream(FILE* handle);
  ~StandardInputStream(); 

  int read();

  int read(char* b, int len);

  void close();

  void setBlocking(bool on) {};
  
protected:
  FILE* handle;
};
#endif // SHABACK_StandardInputStream_H
