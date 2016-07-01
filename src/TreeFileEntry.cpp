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

  // isSplitFile
  isSplitFile = (id.find_first_of(SPLITFILE_ID_INDICATOR) != string::npos);

  // Filename
  if ((until = line.find('\t', from)) == string::npos) throw InvalidTreeFile("Missing filename");
  filename = line.substr(from, until - from);
  from = until +1;

  // Path
  this->parentDir = parentDir;
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

  // Size
  if ((until = line.find('\t', from)) == string::npos) throw InvalidTreeFile("Missing file size");
  n = line.substr(from, until - from);
  from = until +1;
  size = strtoimax(n.c_str(), 0, 10);

  if (type == TREEFILEENTRY_SYMLINK) {
    // Symlink destination
    if ((until = line.find('\t', from)) == string::npos) {
      symLinkDest = line.substr(from);
    } else {
      symLinkDest = line.substr(from, until - from);
    }
    if (symLinkDest.empty()) throw InvalidTreeFile("Missing symlink destination");
    from = until +1;
  }
}


bool TreeFileEntry::isDirectory()
{
  return (type == TREEFILEENTRY_DIRECTORY);
}
