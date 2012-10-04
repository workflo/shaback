/*
 * shaback - A hash digest based backup tool.
 * Copyright (C) 2012 Florian Wolff (florian@donuz.de)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
# include <utime.h>
# include <unistd.h>
# include <sys/acl.h>
# include <acl/libacl.h>
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

File::File() : path("."), initialized(false) {}

File::File(string path) :
  path(path), initialized(false)
{
  canonicalize();
  fname = path.substr(path.rfind("/") + 1);
}

File::File(File parent, string filename) : initialized(false)
{
  path = parent.path;
  path.append("/").append(filename);
  canonicalize();
  fname = path.substr(path.rfind("/") + 1);
}

File::~File()
{
}

File File::home()
{
  string path;

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

  return File(path);
}

File File::tmpdir()
{
  string path;

#ifdef WIN32

  // TODO: WIN32: File::tmpdir

#else

  char* p = getenv("SHABACK_TMP");
  if (!p) p = getenv("TMPDIR");
  if (!p) p = getenv("TMP");
  if (!p) p = getenv("TEMP");
  if (!p) p = "/tmp";

  return File(p);

#endif
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
  return fileExists && S_ISREG(this->statBuffer.st_mode);
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

bool File::mkdirs()
{
  if (this->path == ".") return false;

  initialized = false;
  File parent = getParent();

  if (!parent.isDir()) {
    parent.mkdirs();
  }

#ifdef WIN32
  return (::_mkdir(path.c_str()) == 0);
#else
  return (::mkdir(path.c_str(), 0777) == 0);
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
#ifdef XATTR_NOFOLLOW
  return (setxattr(path.c_str(), key.c_str(), value.data(), value.size(), 0, XATTR_NOFOLLOW) == 0);
#else
  return (setxattr(path.c_str(), key.c_str(), value.data(), value.size(), 0) == 0);
#endif
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
#ifdef XATTR_NOFOLLOW
  return (setxattr(path.c_str(), key.c_str(), str, strlen(str), 0, XATTR_NOFOLLOW) == 0);
#else
  return (setxattr(path.c_str(), key.c_str(), str, strlen(str), 0) == 0);
#endif
#endif
}

std::string File::getXAttr(string key)
{
#ifdef WIN32
  return string();
#else
  char buf[100];
#ifdef XATTR_NOFOLLOW
  int len = getxattr(path.c_str(), key.c_str(), buf, 100, 0, XATTR_NOFOLLOW);
#else
  int len = getxattr(path.c_str(), key.c_str(), buf, 100);
#endif
  if (len >= 0) {
    return string(buf, len);
  } else {
    return string();
  }
#endif
}

File File::getParent()
{
  int lastSlash = path.rfind("/");
  if (lastSlash == string::npos) {
    throw FileNotFoundException(string(path).append("/.."));
  }
//  cout << "getParent: " << path.substr(0, lastSlash)<<endl;
  return File(path.substr(0, lastSlash));
}

void File::canonicalize()
{
  // Remove all trailing slashes:
  while (path.size() > 1 && path.at(path.size() -1) == '/') {
    path.erase(path.size() -1);
  }

  // Remove duplicate slashes:
  int pos;
  while ((pos = path.find("//")) != string::npos) {
    path.erase(pos, 1);
  }
}

void File::chmod(int mode)
{
  int ret = ::chmod(path.c_str(), mode);
  if (ret != 0)
    throw Exception::errnoToException(path);
}

void File::chown(int uid, int gid)
{
  int ret = ::chown(path.c_str(), uid, uid);
  if (ret != 0)
    throw Exception::errnoToException(path);
}

bool File::remove()
{
  int ret = ::remove(path.c_str());
  return (ret == 0);
}

void File::utime(int mtime)
{
  struct utimbuf tm;

  tm.actime = mtime;
  tm.modtime = mtime;

  int ret = ::utime(path.c_str(), &tm);
  if (ret != 0)
    throw Exception::errnoToException(path);
}

bool filePathComparator(File a,File b)
{
  return (a.path < b.path);
}


string File::getAclString()
{
  acl_t acl = acl_get_file(path.c_str(), ACL_TYPE_ACCESS);

  if (acl != 0) {
    char* _text = acl_to_any_text(acl, 0, '|', 0);
    if (_text != 0) {
      string text(_text);

      acl_free(_text);
      acl_free(acl);
      return text;
    }

    acl_free(acl);
  }

  return "";
}

void File::setAcl(string aclString)
{
  acl_t acl = acl_from_text(aclString.c_str());

  if (acl == 0)
    throw Exception::errnoToException(aclString);

//  cout << "File::setAcl: file=" << path << ", acl=" << aclString << endl;

  int ret = acl_set_file(path.c_str(), ACL_TYPE_ACCESS, acl);

  acl_free(acl);

  if (ret != 0)
    throw Exception::errnoToException(path);
}
