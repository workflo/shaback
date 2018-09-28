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
#include <inttypes.h>
 
#include "lib/File.h"

#include "ShabackException.h"
#include "TreeFileEntry.h"
#include "Repository.h"

using namespace std;

TreeFileEntry::TreeFileEntry(string& line, string& parentDir)
{
  if (line.size() < 10)
    throw InvalidTreeFile("Invalid directory file: Empty/short line");

  int from = 2;
  int until;

  // Entry Type
  type = line.at(0);
  if (type != TREEFILEENTRY_FILE && type != TREEFILEENTRY_DIRECTORY && type != TREEFILEENTRY_SYMLINK)
    throw InvalidTreeFile(string("Invalid directory file: Invalid entry type `").append(&type, 1).append("'"));
  if (line.at(1) != '\t')
    throw InvalidTreeFile("Invalid directory file: Entry type should be followed by <TAB>");

  // ID
  if ((until = line.find('\t', from)) == string::npos) throw InvalidTreeFile("Invalid directory file: Missing file ID");
  id = line.substr(from, until - from);
  from = until +1;

  // isSplitFile
  isSplitFile = (id.find_first_of(SPLITFILE_ID_INDICATOR) != string::npos);

  // Filename
  if ((until = line.find('\t', from)) == string::npos) throw InvalidTreeFile(string("Invalid directory file: Missing filename in line \"").append(line).append("\""));
  filename = line.substr(from, until - from);
  from = until +1;

  // Path
  this->parentDir = parentDir;
  path = parentDir;
  if (!path.empty()) path.append(File::separator);
  path.append(filename);

  // File mode (POSIX)
  if ((until = line.find('\t', from)) == string::npos) throw InvalidTreeFile("Invalid directory file: Missing file mode octets");
  string n = line.substr(from, until - from);
  from = until +1;
  fileMode = strtol(n.c_str(), 0, 8);

  // UID (POSIX)
  if ((until = line.find('\t', from)) == string::npos) throw InvalidTreeFile("Invalid directory file: Missing file owner uid");
  n = line.substr(from, until - from);
  from = until +1;
  uid = strtol(n.c_str(), 0, 10);

  // GID (POSIX)
  if ((until = line.find('\t', from)) == string::npos) throw InvalidTreeFile("Invalid directory file: Missing file owner gid");
  n = line.substr(from, until - from);
  from = until +1;
  gid = strtol(n.c_str(), 0, 10);

  // MTime (POSIX)
  if ((until = line.find('\t', from)) == string::npos) throw InvalidTreeFile("Invalid directory file: Missing file mtime");
  n = line.substr(from, until - from);
  from = until +1;
  mtime = strtol(n.c_str(), 0, 10);

  // CTime (POSIX)
  if ((until = line.find('\t', from)) == string::npos) throw InvalidTreeFile("Invalid directory file: Missing file ctime");
  n = line.substr(from, until - from);
  from = until +1;
  ctime = strtol(n.c_str(), 0, 10);

  // Size
  if ((until = line.find('\t', from)) == string::npos) throw InvalidTreeFile("Invalid directory file: Missing file size");
  n = line.substr(from, until - from);
  from = until +1;
  if (type == TREEFILEENTRY_FILE) {
    size = strtoimax(n.c_str(), 0, 10);
  } else {
    size = 0;
  }

  if (type == TREEFILEENTRY_SYMLINK) {
    // Symlink destination
    if ((until = line.find('\t', from)) == string::npos) {
      symLinkDest = line.substr(from);
    } else {
      symLinkDest = line.substr(from, until - from);
    }
    if (symLinkDest.empty()) throw InvalidTreeFile("Invalid directory file: Missing symlink destination");
    from = until +1;
  }
}


TreeFileEntry::TreeFileEntry()
    : type(0)
{}


bool TreeFileEntry::isDirectory()
{
  return (type == TREEFILEENTRY_DIRECTORY);
}


bool TreeFileEntry::isEof()
{
  return (type == TREEFILEENTRY_EOF);
}

string TreeFileEntry::toString()
{
  string s;
  char buf[100];
  
  switch (type) {
    case TREEFILEENTRY_DIRECTORY: {
      sprintf(buf, "\t%03o\t%d\t%d\t%d\t%d\t%i\t\n", fileMode, uid, gid, mtime, ctime, 0);

      s.append("D\t\t").append(path).append(buf);
      break;
    }

    case TREEFILEENTRY_FILE: {
      sprintf(buf, "\t%03o\t%d\t%d\t%d\t%d\t%jd\t\n", fileMode, uid, gid, mtime, ctime, size);

      s.append("F\t").append(id).append("\t").append(path).append(buf);
      break;
    }
    
    case TREEFILEENTRY_SYMLINK: {
      sprintf(buf, "\t%03o\t%d\t%d\t%d\t%d\t\t", fileMode, uid, gid, mtime, ctime);

      s.append("S\t*\t").append(path).append(buf).append(symLinkDest).append("\n");
      break;
    }
  }
  return s;
}
