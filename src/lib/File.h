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

#ifndef SHABACK_File_H
#define SHABACK_File_H

#include <string>
#include <vector>
#include <sys/stat.h>
#include <stdio.h>

#ifdef WIN32
# include <windows.h>
# include <direct.h>
#endif
#include "config.h"

#ifdef PATH_MAX
# define MAX_PATH_LEN PATH_MAX
#else
# define MAX_PATH_LEN FILENAME_MAX
#endif

class File
{
  public:
    File();
    File(std::string path);
    File(File parent, std::string filename);
    virtual ~File();

    /**
     * Returns a new File instance representing the user's
     * home directory.
     */
    static File home();

    /**
     * Returns a new File instance representing the user's
     * TMP directory.
     */
    static File tmpdir();

    virtual void refresh();
    virtual bool isFile();
    virtual bool isDir();
    virtual bool isSymlink();
    virtual bool exists();
    virtual bool mkdir();
    virtual bool mkdirs();
    virtual std::string readlink();
    virtual std::vector<File> listFiles(std::string pattern);
    virtual std::string listFilesToString(std::string pattern, std::string delimiter);
    virtual bool move(File& destination);
    bool setXAttr(std::string key, std::string value);
    bool setXAttr(std::string key, int value);
    std::string getXAttr(std::string key);
    bool removeXAttr(std::string key);
    virtual File getParent();
    virtual bool remove();

    void chmod(int mode);
    void chown(int uid, int gid);
    void lchown(int uid, int gid);
    void utime(int mtime);

    std::string path;
    std::string fname;

    static char separatorChar;
    static std::string separator;

    inline int getPosixMode()
    {
      assertInitialized();
#ifdef WIN32
      return -1;
#else
      return statBuffer.st_mode;
#endif
    }

    inline int getPosixUid()
    {
      assertInitialized();
#ifdef WIN32
      return -1;
#else
      return statBuffer.st_uid;
#endif
    }

    inline int getPosixGid()
    {
      assertInitialized();
#ifdef WIN32
      return -1;
#else
      return statBuffer.st_gid;
#endif
    }

    inline int getPosixMtime()
    {
      assertInitialized();
#ifdef WIN32
      return -1;
#else
      return (int) statBuffer.st_mtime;
#endif
    }

    inline int getPosixCtime()
    {
      assertInitialized();
#ifdef WIN32
      return -1;
#else
      return (int) statBuffer.st_ctime;
#endif
    }

    inline long long getSize()
    {
      assertInitialized();
#ifdef WIN32
      return -1;
#else
      return statBuffer.st_size;
#endif
    }

    inline dev_t getPosixDev()
    {
      assertInitialized();
#ifdef WIN32
      return -1;
#else
      return statBuffer.st_dev;
#endif
    }

    std::string getName();
    std::string getBasename(std::string suffix);

  protected:
    void canonicalize();

  private:
    bool initialized;
    bool fileExists;
    std::string hashValue;

    void assertInitialized();

#ifdef WIN32
    WIN32_FIND_DATAA ffblk;
#else
  #if defined(__APPLE__) || !defined(HAVE_LSTAT64)
    struct stat statBuffer;
  #else
    struct stat64 statBuffer;
  #endif
#endif
};

bool filePathComparator(File a,File b);

#endif // SHABACK_File_H

