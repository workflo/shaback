#include <iostream>
//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>

#include "lib/File.h"

#include "ShabackException.h"
#include "TreeFileEntry.h"

using namespace std;

TreeFileEntry::TreeFileEntry(string& line, string& parentDir)
{
  if (line.size() < 10)
    throw InvalidTreeFile("Tree file contains empty/short line");

  int from = 2;
  int until;

  // Entry Type
  type = line.at(0);
  if (type != TREEFILEENTRY_FILE && type != TREEFILEENTRY_DIRECTORY && type != TREEFILEENTRY_SYMLINK)
    throw InvalidTreeFile(string("Tree file contains invalid entry type `").append(&type, 1).append("'"));
  if (line.at(1) != '\t')
    throw InvalidTreeFile("Entry type should be followed by <TAB>");

  // ID
  if ((until = line.find('\t', from)) == string::npos) throw InvalidTreeFile("Missing file ID");
  id = line.substr(from, until - from);
  from = until +1;

  // Filename
  if ((until = line.find('\t', from)) == string::npos) throw InvalidTreeFile("Missing filename");
  filename = line.substr(from, until - from);
  from = until +1;

  // Path
  path = parentDir;
  if (!path.empty()) path.append(File::separator);
  path.append(filename);

  // File mode (POSIX)
  if ((until = line.find('\t', from)) == string::npos) throw InvalidTreeFile("Missing file mode octets");
  string n = line.substr(from, until - from);
  from = until +1;
  fileMode = strtol(n.c_str(), 0, 8);

  // UID (POSIX)
  if ((until = line.find('\t', from)) == string::npos) throw InvalidTreeFile("Missing file owner uid");
  n = line.substr(from, until - from);
  from = until +1;
  uid = strtol(n.c_str(), 0, 10);

  // GID (POSIX)
  if ((until = line.find('\t', from)) == string::npos) throw InvalidTreeFile("Missing file owner gid");
  n = line.substr(from, until - from);
  from = until +1;
  gid = strtol(n.c_str(), 0, 10);

  // MTime (POSIX)
  if ((until = line.find('\t', from)) == string::npos) throw InvalidTreeFile("Missing file mtime");
  n = line.substr(from, until - from);
  from = until +1;
  mtime = strtol(n.c_str(), 0, 10);

  // CTime (POSIX)
  if ((until = line.find('\t', from)) == string::npos) throw InvalidTreeFile("Missing file ctime");
  n = line.substr(from, until - from);
  from = until +1;
  ctime = strtol(n.c_str(), 0, 10);
}
