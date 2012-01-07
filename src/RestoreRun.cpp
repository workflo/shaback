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
  repository.lock();
}

RestoreRun::~RestoreRun()
{
  repository.unlock();
}

void RestoreRun::restore(string& treeId, File& destinationDir)
{
  //  if (config.verbose)
  //    cout << "Restoring tree " << treeId << " to " << destinationDir.path << endl;

  vector<TreeFileEntry> treeList = repository.loadTreeFile(treeId);

  // Create parent directory:
  if (!treeList.empty() && !treeList.at(0).parentDir.empty()) {
    File(destinationDir, treeList.at(0).parentDir).mkdirs();
  }

  for (vector<TreeFileEntry>::iterator it = treeList.begin(); it < treeList.end(); it++) {
    TreeFileEntry entry(*it);

    //    printf("RestoreRun: %40s  %-40s  %7o %5d %5d %11d %11d\n", entry.id.c_str(), entry.path.c_str(), entry.fileMode,
    //        entry.uid, entry.gid, entry.mtime, entry.ctime);

    switch (entry.type) {
      case TREEFILEENTRY_DIRECTORY: {
        File dir(destinationDir, entry.path);
        if (config.verbose)
          cout << "[d] " << dir.path << endl;
        dir.mkdirs();
        restore(entry.id, destinationDir);

        restoreMetaData(dir, entry);
        break;
      }

      case TREEFILEENTRY_FILE: {
        File file(destinationDir, entry.path);

        if (config.verbose)
          cout << "[f] " << file.path << endl;

        file.remove();

        FileOutputStream out(file);
        repository.exportFile(entry.id, out);
        out.close();

        restoreMetaData(file, entry);
        numFilesRestored++;
        break;
      }

      case TREEFILEENTRY_SYMLINK: {
        File file(destinationDir, entry.path);

        if (config.verbose)
          cout << "[s] " << file.path << endl;
        repository.exportSymlink(entry, file);

        restoreMetaData(file, entry);
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
    file.chmod(entry.fileMode);
  } catch (Exception& ex) {
    reportError(string("chmod: ").append(ex.getMessage()));
  }

  try {
    file.chown(entry.uid, entry.gid);
  } catch (Exception& ex) {
    reportError(string("chown: ").append(ex.getMessage()));
  }

  if (entry.type != TREEFILEENTRY_SYMLINK) {
    try {
      file.utime(entry.mtime);
    } catch (Exception& ex) {
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
