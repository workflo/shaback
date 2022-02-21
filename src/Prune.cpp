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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lib/Exception.h"
#include "lib/FileInputStream.h"
#include "lib/FileOutputStream.h"

#include "Prune.h"
#include "ShabackException.h"
#include "SplitFileIndexReader.h"
#include "DirectoryFileReader.h"

using namespace std;

Prune::Prune(RuntimeConfig& config, Repository& repository) :
    repository(repository), config(config), filesDeleted(0)
{
}

Prune::~Prune()
{
  repository.unlock();
}

void Prune::run()
{
  repository.lock(true);
  repository.open();

  int deleted = 0;

  if (config.all) {
    set<string> backupNames;
    vector<File> indexFiles = config.indexDir.listFiles("*_????" "-??" "-??_??????.shabackup");

    for (vector<File>::iterator it = indexFiles.begin(); it < indexFiles.end(); it++) {
      File file(*it);
      string name(file.getName());
      name = name.substr(0, name.size() - 28);
      backupNames.insert(name);
    }

    vector<string> backupNamesVector;

    for (std::set<string>::iterator it = backupNames.begin(); it != backupNames.end(); ++it) {
      string backupName(*it);
      if (config.verbose) {
        cout << "Pruning backup set "<< backupName << endl;
      }
      deleted += repository.deleteOldIndexFiles(backupName, config.dryRun);
    }

  } else {
    deleted += repository.deleteOldIndexFiles(config.backupName, config.dryRun);
  }

  if (config.showTotals) {
    cerr << config.color_stats << config.style_bold;
    fprintf(stderr, "Index files deleted: %3d\n", deleted);

    cerr << config.style_default;
  }
}
