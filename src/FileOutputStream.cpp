#include <fcntl.h>

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
// #if defined(JAKELIB_WIN32API)

//   // FIXME: append
//   handle = CreateFile(JAKELIB_LATIN1(filename), GENERIC_WRITE,
//                       0, NULL, CREATE_ALWAYS,
//                       FILE_ATTRIBUTE_NORMAL, NULL);
  
//   if (handle == INVALID_HANDLE_VALUE) {
//     throw IOException(System::explainErrorCode(GetLastError())
//                       .. JAKELIB_AT2("jakelib.io.FileOutputStream.init"));
//   }
  
// #else

  handle = open(filename.c_str(), 
                O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC),
		S_IRUSR | S_IWUSR | S_IRGRP);
  
  if (handle == -1) {
    throw Exception::errnoToException(filename);
  }

// #endif
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
  
// #if defined(JAKELIB_WIN32API)

//   DWORD written;

//   if (!WriteFile(handle, &b[offset], len, &written, NULL)) {
//     throw new IOException(System::explainErrorCode(GetLastError())
//                       .. JAKELIB_AT2("jakelib.io.FileOutputStream.write"));
//   }

// #else

  if (::write(handle, b, len) != len) {
    throw Exception::errnoToException();
  }

// #endif
}


/*****************************************************************************\
 * close                                                                      |
 *****************************************************************************/
void FileOutputStream::close()
{
// #if defined(JAKELIB_WIN32API)

//   if (handle != INVALID_HANDLE_VALUE) {
//     CloseHandle(handle);
//     handle = INVALID_HANDLE_VALUE;
//   }

// #else

  if (handle != -1) {
    ::close(handle);
    handle = -1;
  }

// #endif
}

