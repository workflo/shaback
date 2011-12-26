#ifndef File_H
#define File_H

#include <string>
#include <sys/stat.h>
#include <stdio.h>


#ifdef PATH_MAX
#define MAX_PATH_LEN PATH_MAX
#else
#define MAX_PATH_LEN FILENAME_MAX
#endif

class File
{
public:
    File();
    File(const char* path);
    File(std::string& path);
    //File(File& parent, std::string& filename);
    File(File& parent, const char* filename);
    ~File();
    void refresh();
    bool isFile();
    bool isDir();
    bool isSymlink();
    bool exists();
    bool mkdir();
    std::string readlink();

    std::string path;
    std::string fname;

    struct stat statBuffer;

 private:
    bool initialized;
    bool fileExists;
    std::string hashValue;
    
    void assertInitialized();
};

#endif // File_H


