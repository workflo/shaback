#ifndef SHABACK_FileInputStream_H
#define SHABACK_FileInputStream_H

#include <string.h>
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

//         HANDLE handle;
  int handle;
  
  void init(std::string& filename);

};
#endif // SHABACK_FileInputStream_H

