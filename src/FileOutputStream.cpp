#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>


#include "FileOutputStream.h"
#include "Exception.h"

using namespace std;


FileOutputStream::FileOutputStream(File& file, bool append)
{
  this->append = append;
  init(file.path);
}

FileOutputStream::FileOutputStream(const char *filename, bool append)
{
  this->append = append;
  string _filename(filename);
  init(_filename);
}

FileOutputStream::FileOutputStream(string& filename, bool append)
{
  this->append = append;
  init(filename);
}

FileOutputStream::~FileOutputStream()
{
  close();
}


/*****************************************************************************\
 * init                                                                       |
 *****************************************************************************/
void FileOutputStream::init(string& filename)
{
#if defined(WIN32)

   // FIXME: append
  handle = CreateFileA(filename.c_str(), GENERIC_WRITE,
                       0, NULL, CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL, NULL);
  
   if (handle == INVALID_HANDLE_VALUE) {
     throw Exception::errnoToException(filename);
   }
  
#else

  handle = ::open(filename.c_str(), 
                O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC),
		S_IRUSR | S_IWUSR | S_IRGRP);
  
  if (handle == -1) {
    throw Exception::errnoToException(filename);
  }

#endif
}


/*****************************************************************************\
 * write                                                                      |
 *****************************************************************************/
void FileOutputStream::write(int b)
{
  char c = (char) b;
  write(&c, 1);
}


/*****************************************************************************\
 * write                                                                      |
 *****************************************************************************/
void FileOutputStream::write(const char* b, int len)
{
//   if (b == 0)
//     throw new NullPointerException();
//   else if (len < 0)
//     throw new IndexOutOfBoundsException();
//   else if (len == 0)
//     return;
  
#if defined(WIN32)

  DWORD written;

  if (!WriteFile(handle, b, len, &written, NULL)) {
    throw Exception::errnoToException();
  }

#else

  if (::write(handle, b, len) != len) {
    throw Exception::errnoToException();
  }

#endif
}


/*****************************************************************************\
 * close                                                                      |
 *****************************************************************************/
void FileOutputStream::close()
{
#if defined(WIN32)

  if (handle != INVALID_HANDLE_VALUE) {
    CloseHandle(handle);
    handle = INVALID_HANDLE_VALUE;
  }

#else

  if (handle != -1) {
    ::close(handle);
    handle = -1;
  }

#endif
}

