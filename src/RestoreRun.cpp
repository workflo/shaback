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
#include "lib/StandardOutputStream.h"

#include "RestoreRun.h"
#include "ShabackInputStream.h"
#include "DirectoryFileReader.h"

using namespace std;

RestoreRun::RestoreRun(RuntimeConfig& config, Repository& repository, File shabackupFile, bool testRestore) :
    repository(repository), config(config), testRestore(testRestore), shabackupFile(shabackupFile)
{
  repository.lock();
}

RestoreRun::~RestoreRun()
{
  repository.unlock();
}

RestoreReport RestoreRun::start(list<string> files, File& destinationDir)
{
  time(&lastProgressTime);

  DirectoryFileReader dirFileReader(repository, shabackupFile);
  dirFileReader.open();

  // // Open index file to read directory sizes:
  // vector<TreeFileEntry> treeList = repository.loadTreeFile(treeId);

  // for (vector<TreeFileEntry>::iterator it = treeList.begin(); it < treeList.end(); it++) {
  //   TreeFileEntry entry(*it);
  //   report.bytesToBeRestored += entry.size;
  // }

  // if (config.restoreAsCpioStream || config.restoreAsShabackStream) {
  //   restoreAsCpioStream(treeId);
  // } else {
  //   restore(treeId, destinationDir);
  // }

  if (config.showTotals) {
    report.dump();
  }

  return report;
}

void RestoreRun::restore(string& treeId, File& destinationDir, int depth)
{
  vector<TreeFileEntry> treeList = repository.loadTreeFile(treeId);

  for (vector<TreeFileEntry>::iterator it = treeList.begin(); it < treeList.end(); it++) {
    TreeFileEntry entry(*it);

    switch (entry.type) {
      case TREEFILEENTRY_DIRECTORY: {
        if (testRestore) {
          restore(entry.id, destinationDir, depth + 1);
        } else {
          File dir(destinationDir, entry.path);

          bool skip = (config.skipExisting && dir.isDir());

          if (!skip) {
            if (config.verbose && !config.gauge)
              cerr << "[d] " << dir.path << endl;

            dir.mkdirs();
          }
          if (dir.isDir()) {
            restore(entry.id, destinationDir, depth + 1);
            if (!skip) {
              restoreMetaData(dir, entry);
            }
          } else {
            reportError(string("Cannot create destination directory: ").append(dir.path));
          }
        }
        break;
      }

      case TREEFILEENTRY_FILE: {
        if (testRestore) {
          repository.testExportFile(*this, entry);
        } else {
          File file(destinationDir, entry.path);

          if (config.skipExisting && file.isFile())
            break;

          if (config.verbose && !config.gauge) 
            cerr << "[f] " << file.path << endl;

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
            report.numFilesRestored++;
            report.numBytesRestored += entry.size;

            if (!config.quiet) progress(entry.path);
          } catch (Exception &ex) {
            reportError(string("Cannot restore file ").append(file.path).append(": ").append(ex.getMessage()));
          }
        }
        break;
      }

      case TREEFILEENTRY_SYMLINK: {
        if (testRestore) break;

        File file(destinationDir, entry.path);

        if (config.skipExisting && file.isSymlink())
          break;

        if (config.verbose && !config.gauge)
          cout << "[s] " << file.path << endl;

        if (depth == 0)
          file.getParent().mkdirs();

        repository.exportSymlink(entry, file);
        restoreMetaData(file, entry);
        report.numFilesRestored++;
        break;
      }

      default:
        throw IllegalStateException("Unexpected tree file entry type");
    }
  }
}

void RestoreRun::restoreAsCpioStream(string& treeId, int depth)
{
  StandardOutputStream out(stdout);
  vector<TreeFileEntry> treeList = repository.loadTreeFile(treeId);

  for (vector<TreeFileEntry>::iterator it = treeList.begin(); it < treeList.end(); it++) {
    TreeFileEntry entry(*it);

    string path(entry.path);
    if (path == "/") {
      path = ".";
    } else {
      while (path.size() > 1 && path[0] == '/') {
	      path.erase(0, 1);
      }
    }

    switch (entry.type) {
      case TREEFILEENTRY_DIRECTORY: {
        if (config.restoreAsShabackStream) {
          fprintf(stdout, "ShAbAcKsTrEaM1_%06o%06o%06o%06o%06o%06o%011o%06o%016o%s%c", ++report.fileCount, entry.fileMode,
              entry.uid & 0777777, entry.gid & 0777777, 1, 0, (unsigned int) entry.mtime, (unsigned int) path.size()+1, 0,
              path.c_str(), 0x0);
        } else {
          fprintf(stdout, "070707777777%06o%06o%06o%06o%06o%06o%011o%06o%011o%s%c", ++report.fileCount, entry.fileMode,
              entry.uid & 0777777, entry.gid & 0777777, 1, 0, (unsigned int) entry.mtime, (unsigned int) path.size()+1, 0,
              path.c_str(), 0x0);
        }
        restoreAsCpioStream(entry.id, depth + 1);
        break;
      }

      case TREEFILEENTRY_FILE: {
        if (config.restoreAsShabackStream) {
          fprintf(stdout, "ShAbAcKsTrEaM1_%06o%06o%06o%06o%06o%06o%011o%06o%016o%s%c", ++report.fileCount, entry.fileMode,
              entry.uid & 0777777, entry.gid & 0777777, 1, 0, (unsigned int) entry.mtime, (unsigned int) path.size()+1,
              (unsigned int) entry.size, path.c_str(), 0x0);
        } else {
          // cpio has a file size limit of 8 GB :(
          if (entry.size >= CPIO_ODC_MAX_FILE_SIZE) {
            reportError(string("File too large for cpio: ").append(path));
            break;
          }

          fprintf(stdout, "070707777777%06o%06o%06o%06o%06o%06o%011o%06o%011o%s%c", ++report.fileCount, entry.fileMode,
              entry.uid & 0777777, entry.gid & 0777777, 1, 0, (unsigned int) entry.mtime, (unsigned int) path.size()+1,
              (unsigned int) entry.size, path.c_str(), 0x0);
        }
        try {
          repository.exportFile(entry, out);
          report.numFilesRestored ++;
          report.numBytesRestored += entry.size;
        } catch (Exception &ex) {
          reportError(string("Cannot restore file ").append(path).append(": ").append(ex.getMessage()));
        }
        break;
      }

      case TREEFILEENTRY_SYMLINK: {
        if (config.restoreAsShabackStream) {
          fprintf(stdout, "ShAbAcKsTrEaM1_%06o%06o%06o%06o%06o%06o%011o%06o%016o%s%c%s%c", ++report.fileCount, entry.fileMode,
              entry.uid & 0777777, entry.gid & 0777777, 1, 0, (unsigned int) entry.mtime, (unsigned int) path.size()+1,
              (unsigned int) entry.symLinkDest.size() + 1, path.c_str(), 0x0, entry.symLinkDest.c_str(), 0x0);
        } else {
          fprintf(stdout, "070707777777%06o%06o%06o%06o%06o%06o%011o%06o%011o%s%c%s%c", ++report.fileCount, entry.fileMode,
              entry.uid & 0777777, entry.gid & 0777777, 1, 0, (unsigned int) entry.mtime, (unsigned int) path.size()+1,
              (unsigned int) entry.symLinkDest.size() + 1, path.c_str(), 0x0, entry.symLinkDest.c_str(), 0x0);
        }
        break;
      }

      default:
        throw IllegalStateException("Unexpected tree file entry type");
    }
  }

  if (depth == 0) {
    if (config.restoreAsShabackStream) {
      fprintf(stdout, "ShAbAcKsTrEaM1_000000000000000000000000000000000001000000000000000130000000000000000TRAILER!!!%c", 0x0);
    } else {
      fprintf(stdout, "0707070000000000000000000000000000000000010000000000000000000001300000000000TRAILER!!!%c", 0x0);
    }
  }
}

void RestoreRun::restoreMetaData(File& file, TreeFileEntry& entry)
{
  try {
    file.lchown(entry.uid, entry.gid);
  } catch (Exception& ex) {
    if (!config.ignoreErrors.count("chown"))
      reportError(string("chown: ").append(ex.getMessage()));
  }

  if (entry.type == TREEFILEENTRY_FILE) {
    file.setXAttr("user.shaback.sha1", entry.id); // TODO: Use dynamic digest name
    file.setXAttr("user.shaback.mtime", entry.mtime);
  }

  if (entry.type != TREEFILEENTRY_SYMLINK) {
    try {
      file.chmod(entry.fileMode);
    } catch (Exception& ex) {
      if (!config.ignoreErrors.count("chmod"))
        reportError(string("chmod: ").append(ex.getMessage()));
    }

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
  report.numErrors++;
  cerr << "[E] " << msg << endl;
}

void RestoreRun::progress(std::string &path)
{
  time_t now;
  time(&now);

  if (difftime(now, lastProgressTime) >= 1) {
    int percentage = 0;
    if (report.bytesToBeRestored > 0) percentage = min(100.0, (100.0 * (float) report.numBytesRestored / (float) report.bytesToBeRestored));

    if (config.gauge) {
      fprintf(stdout, "XXX\n%d\n%s\nXXX\n", percentage, path.c_str());
      fflush(stdout);
    } else {
      fprintf(stderr, "%jd of %jd bytes (%d%%) restored.\r", report.numBytesRestored, report.bytesToBeRestored, percentage);
    }

    time(&lastProgressTime);
  }
}
