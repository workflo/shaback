/*
 * shaback - A hash digest based backup tool.
 * Copyright (C) 2012-2016 Florian Wolff (florian@donuz.de)
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

#include "lib/Exception.h"
#include "lib/FileInputStream.h"
#include "lib/FileOutputStream.h"

#include "History.h"
#include "ShabackException.h"
#include "SplitFileIndexReader.h"

using namespace std;

History::History(RuntimeConfig& config, Repository& repository) :
    repository(repository), config(config)
{
}

History::~History()
{
  repository.unlock();
}

void History::run()
{
  repository.lock(true);
  repository.open();

  if (config.backupsToKeep > 0) {
    keep(config.backupsToKeep);
  }
  if (config.actionList) {
    list();
  }
}

void History::list()
{
  if (config.all) {
    vector<string> backupNames = listBackupNames();

    for (vector<string>::iterator it = backupNames.begin(); it < backupNames.end(); it++) {
      string backupName(*it);
      list(backupName);
    }
  } else {
    list(config.backupName);
  }
}

void History::list(string& backupName)
{
  vector<File> indexFiles = listIndexFiled(backupName);

  for (vector<File>::iterator it = indexFiles.begin(); it < indexFiles.end(); it++) {
    File file(*it);
    cout << basename(file.path.c_str()) << endl;
  }
}

void History::keep(int backupsToKeep)
{
  vector<File> indexFiles = listIndexFiled(config.backupName);

  int idx = 0;
  for (vector<File>::iterator it = indexFiles.begin(); it < indexFiles.end(); it++) {
    File file(*it);
    if (idx < backupsToKeep) {
      if (config.verbose) cout << "Keeping " << basename(file.path.c_str()) << endl;
    } else {
      if (config.verbose) cout << "Deleting " << basename(file.path.c_str()) << endl;
      file.remove();
    }
    idx++;
  }
}

vector<File> History::listIndexFiled(string& backupName)
{
  string pattern(backupName);
  pattern.append("_????" "-??" "-??_??????.sroot");

  vector<File> indexFiles = config.indexDir.listFiles(pattern);
  sort(indexFiles.begin(), indexFiles.end(), filePathComparator);
  reverse(indexFiles.begin(), indexFiles.end());

  return indexFiles;
}

vector<string> History::listBackupNames()
{
  set<string> backupNames;

  vector<File> indexFiles = config.indexDir.listFiles("*_????" "-??" "-??_??????.sroot");
  for (vector<File>::iterator it = indexFiles.begin(); it < indexFiles.end(); it++) {
    File file(*it);
    string name(basename(file.path.c_str()));
    name = name.substr(0, name.size() - 24);
    backupNames.insert(name);
  }

  vector<string> backupNamesVector;

  for (std::set<string>::iterator it = backupNames.begin(); it != backupNames.end(); ++it) {
    backupNamesVector.push_back(*it);
  }
  sort(backupNamesVector.begin(), backupNamesVector.end(), filePathComparator);

  return backupNamesVector;
}
