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
#include "lib/FileOutputStream.h"

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

      migrate2to3int(out, treeId);

      out.finish();
    }
  }

  repository.unlock();
  
  updateRepoVersion("3");

  cout << endl << "Run garbage collection to get rid of the now obsolete tree files." << endl;
}


void Migration::migrate2to3int(ShabackOutputStream& out, string& treeId)
{
  vector<TreeFileEntry> treeList = repository.loadTreeFile(treeId);
  
  for (vector<TreeFileEntry>::iterator it = treeList.begin(); it < treeList.end(); it++) {
    TreeFileEntry entry(*it);

    if (config.verbose > 1) cout << "    " << entry.path << endl;

    string line(entry.toString());
    out.write(line);

    switch (entry.type) {
      case TREEFILEENTRY_DIRECTORY:
        migrate2to3int(out, entry.id);
        break;
    }
  }
}

vector<File> Migration::listRootFiles()
{
  string pattern("*_????" "-??" "-??_??????.sroot");

  vector<File> rootFiles = config.indexDir.listFiles(pattern);

  return rootFiles;
}


void Migration::updateRepoVersion(string newVersion)
{
  repository.lock(true);

  string repoProperties =
      "# Don't modify this file!\n# Loss of data is inevitable!\n\nversion = ";
  repoProperties.append(newVersion);
  repoProperties.append("\ncompression = ");
  repoProperties.append(Repository::compressionToName(repository.compressionAlgorithm)).append("\nencryption = ") .append(
      Repository::encryptionToName(repository.encryptionAlgorithm)).append("\ndigest = SHA1\n");
  repoProperties.append("repoFormat = ").append(Repository::repoFormatToName(repository.repoFormat)).append("\n");
  FileOutputStream os(config.repoPropertiesFile);
  os.write(repoProperties.data(), repoProperties.size());

  repository.unlock();
}
