#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <pwd.h>

#include "File.h"
#include "Sha1.h"
#include "Exception.h"
#include "FileInputStream.h"

using namespace std;


File::File()
  :path("")
{
  this->initialized = false;
  struct passwd *pw = getpwuid(getuid());
  if (pw) {
    path = pw->pw_dir;
  }
}

File::File(const char* path)
  :path(path)
{
  this->initialized = false;
  this->fname = this->path.substr(this->path.rfind("/") +1);
}

File::File(string& path)
  :path(path)
{
  this->initialized = false;
  this->fname = path.substr(path.rfind("/") +1);
}

File::File(File& parent, const char* filename)
{
  this->path = parent.path;
  if (parent.path.size() > 0 && parent.path.at(parent.path.size() -1) != '/') {
    this->path.append("/");
  }
  this->path.append(filename);
  this->initialized = false;
  this->fname = path.substr(path.rfind("/") +1);
}

File::~File()
{
}


void File::assertInitialized()
{
  if (!initialized) {
    refresh();
  }
}


void File::refresh()
{
  initialized = true;
 
  if (lstat(this->path.c_str(), &this->statBuffer) == -1) {
    fileExists = false;
  } else {
    fileExists = true;
  }
}


bool File::isFile()
{
  assertInitialized();
  return S_ISREG(this->statBuffer.st_mode);
}


bool File::isDir()
{
  assertInitialized();
  return fileExists && S_ISDIR(this->statBuffer.st_mode);
}


bool File::isSymlink()
{
  assertInitialized();
  return fileExists && S_ISLNK(this->statBuffer.st_mode);
}


bool File::exists()
{
  assertInitialized();
  return fileExists && (S_ISREG(this->statBuffer.st_mode) || S_ISDIR(this->statBuffer.st_mode) || S_ISLNK(this->statBuffer.st_mode));
}


bool File::mkdir()
{
  initialized = false;
  return (::mkdir(this->path.c_str(), 0777) == 0);
}


string File::readlink()
{
  char destBuf[MAX_PATH_LEN];
  int len = ::readlink(path.c_str(), destBuf, MAX_PATH_LEN);

  if (len == -1) {
    throw Exception::errnoToException(path);
  }

  return string(destBuf, len);
}
