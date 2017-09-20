/*
 * shaback - A hash digest based backup tool.
 * Copyright (C) 2017 Florian Wolff (florian@donuz.de)
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
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
   
#include "shaback.h"
#include "Migration.h"
#include "TreeFile.h"
#include "lib/Exception.h"
#include "lib/FileInputStream.h"

using namespace std;


Migration::Migration(RuntimeConfig& config, Repository& repository) :
    repository(repository), config(config)
{
}
 
void Migration::run()
{
  repository.open();

  if (repository.version == SHABACK_REPO_VERSION) {
    cerr << "No need to migrate repository." << endl;
  } else if (repository.version == "2") {
    cout << "Migrating repository from version \"" << repository.version << "\" to \"3\"..." << endl;
    migrate2to3();
  } else {
    throw Exception(string("Unsupported repository version \"").append(repository.version).append("\"."));
  }
}

void Migration::migrate2to3()
{
  repository.lock();
  
  vector<File> rootFiles = listRootFiles();
  // char sizeBuf[30];

  for (vector<File>::iterator it = rootFiles.begin(); it < rootFiles.end(); it++) {
    File rootFile(*it);

    string fname = rootFile.getName();
    string bname = fname.substr(0, fname.size() - 6);

    // Read root ID:
    FileInputStream in(rootFile);
    string treeId;
    in.readLine(treeId);

    ShabackOutputStream out(repository.createOutputStream());
    File shabackupFile(config.indexDir, string(bname).append(".shabackup"));

    if (!shabackupFile.isFile()) {
      cout << rootFile.path << endl;

      out.open(shabackupFile);

      out.write(DIRECTORY_FILE_HEADER "\n");

      migrate2to3int(out, treeId, File("/"));

      // out.finalize();
      // rootFile.remove();
    }
  }

  // TODO: Lock exclusively and
  // TODO: Update repo.properties

  repository.unlock();

  cout << "Run garbage collection to get rid of the now obsolete tree files." << endl;
}


void Migration::migrate2to3int(ShabackOutputStream& out, string& treeId, File parentDir)
{
  vector<TreeFileEntry> treeList = repository.loadTreeFile(treeId);
  
  for (vector<TreeFileEntry>::iterator it = treeList.begin(); it < treeList.end(); it++) {
    TreeFileEntry entry(*it);

    cout << "    " << entry.path << endl;

    string line(entry.toString());
    out.write(line);

  //   switch (entry.type) {
  //     case TREEFILEENTRY_DIRECTORY: {
  //       // File dir(destinationDir, entry.path);

  //       // bool skip = (config.skipExisting && dir.isDir());

  //       // if (!skip) {
  //       //   if (config.verbose && !config.gauge)
  //       //     cerr << "[d] " << dir.path << endl;

  //       //   dir.mkdirs();
  //       // }
  //       migrate2to3int(out, entry.id, File(entry.path));
  //       break;
  //     }

  //     case TREEFILEENTRY_FILE: {
  //       // File file(destinationDir, entry.path);

  //       // if (config.skipExisting && file.isFile())
  //       //   break;

  //       // if (config.verbose && !config.gauge) 
  //       //   cerr << "[f] " << file.path << endl;

  //       // // Create base directory:
  //       // if (depth == 0) {
  //       //   file.getParent().mkdirs();
  //       //   if (!file.getParent().isDir()) {
  //       //     reportError(string("Cannot create destination directory: ").append(file.getParent().path));
  //       //   }
  //       // }

  //       break;
  //     }

  //     case TREEFILEENTRY_SYMLINK: {
  //       // File file(destinationDir, entry.path);

  //       // if (config.skipExisting && file.isSymlink())
  //       //   break;

  //       // if (config.verbose && !config.gauge)
  //       //   cout << "[s] " << file.path << endl;

  //       // if (depth == 0)
  //       //   file.getParent().mkdirs();

  //       // repository.exportSymlink(entry, file);
  //       // restoreMetaData(file, entry);
  //       // report.numFilesRestored++;
  //       break;
  //     }

  //     default:
  //       throw IllegalStateException("Unexpected tree file entry type");
  //   }
  }
}

vector<File> Migration::listRootFiles()
{
  string pattern("*_????" "-??" "-??_??????.sroot");

  vector<File> rootFiles = config.indexDir.listFiles(pattern);

  return rootFiles;
}
