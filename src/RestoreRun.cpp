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

RestoreRun::RestoreRun(RuntimeConfig& config, Repository& repository, File shabackupFile,
  bool testRestore, bool listFiles) :
    repository(repository), config(config), testRestore(testRestore), 
    shabackupFile(shabackupFile), listFiles(listFiles)
{
  repository.lock();
}

RestoreRun::~RestoreRun()
{
  repository.unlock();
}

RestoreReport RestoreRun::start(list<string> _files, File& destinationDir)
{
  time(&lastProgressTime);

  DirectoryFileReader dirFileReader(repository, shabackupFile);
  dirFileReader.open();

  vector<string> files;
  for (list<string>::iterator it = _files.begin(); it != _files.end(); it++) {
    string file(*it);
    while (file.size() > 1 && file.at(file.size() -1) == File::separatorChar) file.erase(file.size() -1);
    files.push_back(file);

    // Add filename + "/" to the list:
    if (file.size() > 1) {
      string file2(file);
      file2.append(File::separator);
      files.push_back(file2);
    }
  }

  do {
    TreeFileEntry entry(dirFileReader.next());
    if (entry.isEof()) break;

    for (vector<string>::iterator it = files.begin(); it < files.end(); it++) {
      string file(*it);
      if (entry.path == file || entry.path.substr(0, file.size()) == file) {
        report.bytesToBeRestored += entry.size;
      
        if (config.restoreAsCpioStream || config.restoreAsShabackStream) {
          restoreAsCpioStream(entry);
        } else {
          restore(entry, destinationDir);
        }
        break;
      }
    }
  } while(true);

  if (config.restoreAsShabackStream) {
    fprintf(stdout, "ShAbAcKsTrEaM1_000000000000000000000000000000000001000000000000000130000000000000000TRAILER!!!%c", 0x0);
  } else if (config.restoreAsCpioStream) {
    fprintf(stdout, "0707070000000000000000000000000000000000010000000000000000000001300000000000TRAILER!!!%c", 0x0);
  }
  
  if (config.showTotals) {
    report.dump();
  }

  return report;
}

void RestoreRun::restore(TreeFileEntry& entry, File& destinationDir)
{
  switch (entry.type) {
    case TREEFILEENTRY_DIRECTORY: {
      if (testRestore || listFiles) break;

      File dir(destinationDir, entry.path);

      bool skip = (config.skipExisting && dir.isDir());

      if (!skip) {
        if (config.verbose && !config.gauge)
          cerr << "[d] " << dir.path << endl;

        dir.mkdirs();
        restoreMetaData(dir, entry);
      }
      if (!dir.isDir()) {
        reportError(string("Cannot create destination directory: ").append(dir.path));
      }
      break;
    }

    case TREEFILEENTRY_FILE: {
      if (testRestore) {
        repository.testExportFile(*this, entry);
      } else if (listFiles) {
        cout << repository.hashValueToFile(entry.id).path << endl;
      } else {
        File file(destinationDir, entry.path);

        if (file.isFile()) {
          if (config.skipExisting) {
            break;
          } else {
            file.remove();
          }
        } else {
          file.getParent().mkdirs();
        }

        if (config.verbose && !config.gauge) 
          cerr << "[f] " << file.path << endl;

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
      if (testRestore || listFiles) break;

      File file(destinationDir, entry.path);

      file.getParent().mkdirs();
      
      if (config.skipExisting && file.isSymlink()) break;

      file.getParent().mkdirs();

      if (config.verbose && !config.gauge)
        cout << "[s] " << file.path << endl;

      repository.exportSymlink(entry, file);
      restoreMetaData(file, entry);
      report.numFilesRestored++;
      break;
    }

    default:
      throw IllegalStateException("Unexpected tree file entry type");
  }
}

void RestoreRun::restoreAsCpioStream(TreeFileEntry& entry)
{
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
        StandardOutputStream stdoutStream(stdout);
        repository.exportFile(entry, stdoutStream);
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
