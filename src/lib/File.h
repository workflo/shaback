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

#ifdef PATH_MAX
# define MAX_PATH_LEN PATH_MAX
#else
# define MAX_PATH_LEN FILENAME_MAX
#endif

class File
{
  public:
    File();
    File(const char* path);
    File(std::string& path);
    File(File& parent, std::string& filename);
    File(File& parent, const char* filename);
    ~File();
    void refresh();
    bool isFile();
    bool isDir();
    bool isSymlink();
    bool exists();
    bool mkdir();
    std::string readlink();
    std::vector<File> listFiles(std::string pattern);
    bool move(File& destination);
    bool setXAttr(std::string key, std::string value);
    bool setXAttr(std::string key, int value);
    std::string getXAttr(std::string key);

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

  private:
    bool initialized;
    bool fileExists;
    std::string hashValue;

    void assertInitialized();

#ifdef WIN32
    WIN32_FIND_DATAA ffblk;
#else
    struct stat statBuffer;
#endif
};

#endif // SHABACK_File_H

