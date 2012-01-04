#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>

#ifdef WIN32
# include <windows.h>
# include <direct.h>
# include <Shlobj.h>
#else
# include <dirent.h>
# include <pwd.h>
# include <fnmatch.h>
# include <sys/xattr.h>
#endif

#include "File.h"
#include "Sha1.h"
#include "Exception.h"
#include "FileInputStream.h"

using namespace std;

#ifdef WIN32
char File::separatorChar = '\\';
string File::separator = "\\";
#else
char File::separatorChar = '/';
string File::separator = "/";
#endif

File::File() :
  path("")
{
  this->initialized = false;
#ifdef WIN32
  char tmpbuf[MAX_PATH];
  DWORD size;

  //if (GetCurrentDirectory(JAKELIB_MAX_PATH, tmpbuf) != 0)
  //  properties->setProperty(`"user.dir"`, `""` .. tmpbuf);

  size = MAX_PATH;
  if (SHGetSpecialFolderPathA(NULL, tmpbuf, CSIDL_PERSONAL, false))
  path = tmpbuf;
#else
  struct passwd *pw = getpwuid(getuid());
  if (pw) {
    path = pw->pw_dir;
  }
#endif
}

File::File(const char* path) :
  path(path)
{
  this->initialized = false;
  this->fname = this->path.substr(this->path.rfind("/") + 1);
}

File::File(string& path) :
  path(path)
{
  this->initialized = false;
  this->fname = path.substr(path.rfind("/") + 1);
}

File::File(File& parent, string& filename)
{
  this->path = parent.path;
  if (parent.path.size() > 0 && parent.path.at(parent.path.size() - 1) != '/') {
    this->path.append("/");
  }
  this->path.append(filename);
  this->initialized = false;
  this->fname = path.substr(path.rfind("/") + 1);
}

File::File(File& parent, const char* filename)
{
  this->path = parent.path;
  if (parent.path.size() > 0 && parent.path.at(parent.path.size() - 1) != '/') {
    this->path.append("/");
  }
  this->path.append(filename);
  this->initialized = false;
  this->fname = path.substr(path.rfind("/") + 1);
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

#ifdef WIN32

  HANDLE hFind;

  hFind = FindFirstFileA(path.c_str(), &ffblk);

  if (hFind != INVALID_HANDLE_VALUE) {
    fileExists = true;
    //_isReadOnly = (ffblk.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? true : false;
    //_isHidden = (ffblk.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ? true : false;
    //_isSystem = (ffblk.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) ? true : false;
    //_length = ffblk.nFileSizeLow;
    //_mTime = ((jlong)ffblk.ftLastWriteTime.dwLowDateTime |
    //            ((jlong)ffblk.ftLastWriteTime.dwHighDateTime << 32)) / LL(10000) - LL(11644473600000);
    FindClose(hFind);
  } else {
    fileExists = false;
  }
#else
  if (lstat(path.c_str(), &statBuffer) == -1) {
    fileExists = false;
  } else {
    fileExists = true;
  }
#endif
}

bool File::isFile()
{
  assertInitialized();
#ifdef WIN32
  return (!(ffblk.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) ? true : false;
#else
  return S_ISREG(this->statBuffer.st_mode);
#endif
}

bool File::isDir()
{
  assertInitialized();
#ifdef WIN32
  return (ffblk.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false;
#else
  return fileExists && S_ISDIR(this->statBuffer.st_mode);
#endif
}

bool File::isSymlink()
{
#ifdef WIN32
  return false;
#else
  assertInitialized();
  return fileExists && S_ISLNK(this->statBuffer.st_mode);
#endif
}

bool File::exists()
{
  assertInitialized();
  return fileExists && (isFile() || isDir() || isSymlink());
}

bool File::mkdir()
{
  initialized = false;
#ifdef WIN32
  return (::_mkdir(this->path.c_str()) == 0);
#else
  return (::mkdir(this->path.c_str(), 0777) == 0);
#endif
}

string File::readlink()
{
#ifdef WIN32
  throw UnsupportedOperation("readlink");
#else
  char destBuf[MAX_PATH_LEN];
  int len = ::readlink(path.c_str(), destBuf, MAX_PATH_LEN);

  if (len == -1) {
    throw Exception::errnoToException(path);
  }

  return string(destBuf, len);
#endif
}

vector<File> File::listFiles(string p)
{
  vector<File> list;

  if (!isDir())
    return list;

#ifdef WIN32
  string pattern = path;
  if (path.size() > 0)
  pattern.append(separator);
  pattern.append(p);

  HANDLE dir;
  WIN32_FIND_DATAA dirent;

  if ((dir = FindFirstFileA(pattern.c_str(), &dirent)) != INVALID_HANDLE_VALUE) {
    do {
      if (strcmp(dirent.cFileName, ".") == 0 || strcmp(dirent.cFileName, "..") == 0) continue;
      File f(*this, dirent.cFileName);

      list.push_back(f);
    }
    while(FindNextFileA(dir, &dirent));

    FindClose(dir);
  }
  // TODO: WIN32: Order by filename
#else
  DIR* dir = opendir(path.c_str());

  if (dir) {
    struct dirent* entry;

    while ((entry = readdir(dir))) {
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || fnmatch(p.c_str(), entry->d_name, 0))
        continue;
      File f(*this, entry->d_name);
      list.push_back(f);
    }

    closedir(dir);
  }
#endif

  return list;
}

bool File::move(File& destination)
{
  return (::rename(path.c_str(), destination.path.c_str()) == 0);
}

bool File::setXAttr(string key, string value)
{
#ifdef WIN32
  // Do nothing.
  return false;
#else
  return (setxattr(path.c_str(), key.c_str(), value.data(), value.size(), 0, XATTR_NOFOLLOW) == 0);
#endif
}

bool File::setXAttr(string key, int value)
{
#ifdef WIN32
  // Do nothing.
  return false;
#else
  char str[20];
  sprintf(str, "%d", value);
  return (setxattr(path.c_str(), key.c_str(), str, strlen(str), 0, XATTR_NOFOLLOW) == 0);
#endif
}

std::string File::getXAttr(string key)
{
#ifdef WIN32
  return string();
#else
  char buf[100];
  int len = getxattr(path.c_str(), key.c_str(), buf, 100, 0, XATTR_NOFOLLOW);
  if (len >= 0) {
    return string(buf, len);
  } else {
    return string();
  }
#endif
}
