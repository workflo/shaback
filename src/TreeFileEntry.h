#ifndef SHABACK_TreeFileEntry_H
#define SHABACK_TreeFileEntry_H

#include <string>

#define TREEFILEENTRY_FILE          'F'
#define TREEFILEENTRY_DIRECTORY     'D'
#define TREEFILEENTRY_SYMLINK       'S'

class TreeFileEntry
{
  public:
    TreeFileEntry(std::string& line, std::string& parentDir);

    char type;
    std::string id;
    std::string filename;
    std::string path;
    std::string parentDir;
    int fileMode;
    int uid;
    int gid;
    int mtime;
    int ctime;
    std::string symLinkDest;
};

#endif // SHABACK_TreeFileEntry_H
