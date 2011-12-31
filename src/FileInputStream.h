#ifndef SHABACK_FileInputStream_H
#define SHABACK_FileInputStream_H

#include <string.h>
#ifdef WIN32
#include <windows.h>
#endif
#include "InputStream.h"
#include "File.h"


/**
 * This class is a stream that reads its bytes from a file.
 *
 * @class FileInputStream
 * @author Florian 'Overflo' Wolff (florian@donuz.de)
 */
class FileInputStream : public InputStream
{
public:

  /**
   * Opens the specified file for reading.
   */
  FileInputStream(File& file); 

  /**
   * Opens the specified file for reading.
   */
  FileInputStream(const char* filename); 

  /**
   * Opens the specified file for reading.
   */
  FileInputStream(std::string& filename); 

  ~FileInputStream();

  int read();

  int read(char* b, int len);

  void close();

  void reset();

  int skip(int n);

  int available();

  void setBlocking(bool on) {};
  
protected:
#ifdef WIN32
  HANDLE handle;
#else
  int handle;
#endif

  void init(std::string& filename);

};
#endif // SHABACK_FileInputStream_H

