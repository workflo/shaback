#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>

#include "File.h"
#include "Sha1.h"

using namespace std;


File::File()
  :path(".")
{
  this->initialized = false;
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
  } else {
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
  return S_ISDIR(this->statBuffer.st_mode);
}


bool File::isSymlink()
{
  assertInitialized();
  return S_ISLNK(this->statBuffer.st_mode);
}


bool File::exists()
{
  assertInitialized();
  return S_ISREG(this->statBuffer.st_mode) || S_ISDIR(this->statBuffer.st_mode) || S_ISLNK(this->statBuffer.st_mode);
}


bool File::mkdir()
{
  initialized = false;
  return (::mkdir(this->path.c_str(), 0777) == 0);
}


#define READ_BUFFER_SIZE (1024 * 4)
static char readBuffer[READ_BUFFER_SIZE];

std::string File::getHashValue()
{
  if (hashValue.empty()) {
    Sha1 sha1;
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == 0) {
      // TODO: Fehler!
    }
    while (true) {
      ssize_t bytesRead = read(fd, readBuffer, READ_BUFFER_SIZE);
      if (bytesRead == -1) {
	// TODO: Fehler!
	break;
      } else if (bytesRead == 0) {
	break;
      }
      sha1.update(readBuffer, bytesRead);
    }
  
    close(fd);
    sha1.finalize();
    hashValue = sha1.toString();
  }
  
  return hashValue;
}


void File::copyTo(File& destFile)
{
  int fdIn = open(path.c_str(), O_RDONLY);
  if (fdIn == 0) {
    // TODO: Fehler!
    return;
  }

  int fdOut = ::open(destFile.path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0777);
  if (fdOut == 0) {
    // TODO: Fehler
    return;
  }

  while (true) {
    ssize_t bytesRead = read(fdIn, readBuffer, READ_BUFFER_SIZE);
    if (bytesRead == -1) {
      // TODO: Fehler!
      break;
    } else if (bytesRead == 0) {
      break;
    }
    write(fdOut, readBuffer, bytesRead);
  }

  close(fdOut);
  close(fdIn);
}


string File::readlink()
{
  char destBuf[MAX_PATH_LEN];
  int len = ::readlink(path.c_str(), destBuf, MAX_PATH_LEN);

  if (len == -1) {
    // TODO: Fehler
    return "";
  }

  return string(destBuf, len);
}
