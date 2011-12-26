#include <fcntl.h>

#include "FileInputStream.h"
#include "Exception.h"

using namespace std;


/*****************************************************************************\
 * FileInputStream                                                            |
 *****************************************************************************/
FileInputStream::FileInputStream(File& file)
{
  init(file.path);
}

FileInputStream::FileInputStream(const char* filename)
{
  string _filename(filename);
  init(_filename);
}

FileInputStream::FileInputStream(string& filename)
{
  init(filename);
}

FileInputStream::~FileInputStream()
{
  close();
}


/*****************************************************************************\
 * init                                                                       |
 *****************************************************************************/
void FileInputStream::init(string& filename)
{
// #if defined(JAKELIB_WIN32API)

//   handle = CreateFile(JAKELIB_LATIN1(filename), GENERIC_READ,
//                       FILE_SHARE_READ, NULL, OPEN_EXISTING,
//                       FILE_ATTRIBUTE_NORMAL, NULL);
  
//   if (handle == INVALID_HANDLE_VALUE) {
//     throw new FileNotFoundException(System::explainErrorCode(GetLastError())
//                                     .. JAKELIB_AT2("jakelib.io.FileInputStream.init"),
//                                     filename);
//   }

// #else

  handle = open(filename.c_str(), O_RDONLY);
   
  if (handle == -1) {
    throw Exception::errnoToException(filename);
  }

// #endif
}


/*****************************************************************************\
 * read                                                                       |
 *****************************************************************************/
int FileInputStream::read()
{
  char b;
  long r;
  r = read(&b, 1);
  if (r == 1)
    return b & 0xff;
  else 
    return -1;
}


/*****************************************************************************\
 * read                                                                       |
 *****************************************************************************/
int FileInputStream::read(char* b, int len)
{
//   if (b == null)
//     throw new NullPointerException(JAKELIB_AT2("jakelib.io.FileInputStream.read"));
//   else if (len < 0 || offset < 0)
//     throw new IndexOutOfBoundsException(JAKELIB_AT2("jakelib.io.FileInputStream.read"));
//   else if (len == 0)
//     return 0;
  
// #if defined(JAKELIB_WIN32API)

//   DWORD r;
  
//   if (!ReadFile(handle, &b[offset], len, &r, NULL)) {
//     throw new IOException(System::explainErrorCode(GetLastError())
//                       .. JAKELIB_AT2("jakelib.io.FileInputStream.read"));
//   }

//   if (r == 0)
//     return -1;
//   else
//     return r;

// #else

  int r;

  if ((r = ::read(handle, (char*) b, len)) < 0) {
    throw Exception::errnoToException();
  }

  if (r == 0)
    return -1;
  else
    return r;

// #endif
}


/*****************************************************************************\
 * close                                                                      |
 *****************************************************************************/
void FileInputStream::close()
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


/*****************************************************************************\
 * reset                                                                      |
 *****************************************************************************/
void FileInputStream::reset()
{
// #if defined(JAKELIB_WIN32API)
//   // FIXME
// #else

  lseek(handle, 0, SEEK_SET);

// #endif
}


/*****************************************************************************\
 * available                                                                  |
 *****************************************************************************/
int FileInputStream::available()
{
  // FIXME
  return 0;
}


/*****************************************************************************\
 * skip                                                                       |
 *****************************************************************************/
int FileInputStream::skip(int n)
{
  // FIXME
  return 0;
}

