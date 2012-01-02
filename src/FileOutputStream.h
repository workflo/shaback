#ifndef SHABACK_FileOutputStream_H
#define SHABACK_FileOutputStream_H

#include <string.h>
#ifdef WIN32
# include <windows.h>
#endif
#include "File.h"
#include "OutputStream.h"

/**
 * This classes allows a stream of data to be written to a disk file.
 *
 * @class FileOutputStream
 * @author Florian 'Overflo' Wolff (florian@donuz.de)
 */
class FileOutputStream : public OutputStream
{
 public:
  FileOutputStream(File& file, bool append = false);
  FileOutputStream(const char* filename, bool append = false); 
  FileOutputStream(std::string&, bool append = false); 
  ~FileOutputStream(); 
  
  void write(int b);
  void write(const char* b, int len);
  void close();
  
 protected:
  bool append;
#ifdef WIN32
  HANDLE handle;
#else
  int handle;
#endif
  void init(std::string& filename);
};
#endif // SHABACK_FileOutputStream_H
