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
#include "lib/FileOutputStream.h"

#include "RestoreRun.h"
#include "ShabackInputStream.h"

using namespace std;

RestoreRun::RestoreRun(RuntimeConfig& config, Repository& repository) :
  repository(repository), config(config), numErrors(0), numFilesRestored(0), numBytesRestored(0)
{
  // TODO Move out of constructor!!
  repository.lock();
}

RestoreRun::~RestoreRun()
{
  repository.unlock();
}

void RestoreRun::restore(string& treeId, File& destinationDir, int depth)
{
  vector<TreeFileEntry> treeList = repository.loadTreeFile(treeId);

  for (vector<TreeFileEntry>::iterator it = treeList.begin(); it < treeList.end(); it++) {
    TreeFileEntry entry(*it);

    switch (entry.type) {
      case TREEFILEENTRY_DIRECTORY: {
        File dir(destinationDir, entry.path);
        if (config.verbose)
          cout << "[d] " << dir.path << endl;
        dir.mkdirs();
        if (dir.isDir()) {
          restore(entry.id, destinationDir, depth +1);
          restoreMetaData(dir, entry);
        } else {
          reportError(string("Cannot create destination directory: ").append(dir.path));
        }
        break;
      }

      case TREEFILEENTRY_FILE: {
        File file(destinationDir, entry.path);

        if (config.verbose)
          cout << "[f] " << file.path << endl;

        // Create base directory:
        if (depth == 0) {
          file.getParent().mkdirs();
          if (!file.getParent().isDir()) {
            reportError(string("Cannot create destination directory: ").append(file.getParent().path));
          }
        }
        file.remove();

        try {
          FileOutputStream out(file);
          repository.exportFile(entry, out);
          out.close();
          restoreMetaData(file, entry);
          numFilesRestored++;
        } catch (Exception &ex) {
          reportError(string("Cannot restore file ").append(file.path).append(": ").append(ex.getMessage()));
        }
        break;
      }

      case TREEFILEENTRY_SYMLINK: {
        File file(destinationDir, entry.path);

        if (config.verbose)
          cout << "[s] " << file.path << endl;

        if (depth == 0)
          file.getParent().mkdirs();

        repository.exportSymlink(entry, file);
        numFilesRestored++;
        break;
      }

      default:
        throw IllegalStateException("Unexpected tree file entry type");
    }
  }
}

void RestoreRun::restoreMetaData(File& file, TreeFileEntry& entry)
{
  try {
    file.chown(entry.uid, entry.gid);
  } catch (Exception& ex) {
    if (!config.ignoreErrors.count("chown"))
      reportError(string("chown: ").append(ex.getMessage()));
  }

  try {
    file.chmod(entry.fileMode);
  } catch (Exception& ex) {
    if (!config.ignoreErrors.count("chmod"))
      reportError(string("chmod: ").append(ex.getMessage()));
  }

  if (entry.type != TREEFILEENTRY_SYMLINK) {
    try {
      file.utime(entry.mtime);
    } catch (Exception& ex) {
      if (!config.ignoreErrors.count("utime"))
        reportError(string("utime: ").append(ex.getMessage()));
    }
  }
}

void RestoreRun::reportError(string msg)
{
  numErrors++;
  cerr << "[E] " << msg << endl;
}

void RestoreRun::showTotals()
{
  printf("Files restored:   %12d\n", numFilesRestored);
  //  #ifdef __APPLE__
  //  printf("Bytes restored:   %12jd\n", (intmax_t) numBytesRestored);
  //  #else
  //  printf("Bytes restored:   %12jd\n", numBytesRestored);
  //  #endif
  printf("Errors:           %12d\n", numErrors);
}
