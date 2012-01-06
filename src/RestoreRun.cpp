#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lib/Exception.h"

#include "RestoreRun.h"
#include "ShabackInputStream.h"
//#include "TreeFileEntry.h"

using namespace std;

RestoreRun::RestoreRun(RuntimeConfig& config, Repository& repository) :
  repository(repository), config(config)
{
  repository.lock();
  numFilesRestored = 0;
  numBytesRestored = 0;
  numErrors = 0;
}

RestoreRun::~RestoreRun()
{
  repository.unlock();
}

void RestoreRun::restore(string& treeId, File& destinationDir)
{
  if (config.verbose)
    cout << "Restoring tree " << treeId << " to " << destinationDir.path << endl;

  vector<TreeFileEntry> treeList = repository.loadTreeFile(treeId);
  for (vector<TreeFileEntry>::iterator it = treeList.begin(); it < treeList.end(); it++) {
    TreeFileEntry entry(*it);

    printf("RestoreRun: %40s  %-40s  %7o %5d %5d %11d %11d\n", entry.id.c_str(), entry.path.c_str(), entry.fileMode,
        entry.uid, entry.gid, entry.mtime, entry.ctime);
	
    if (entry.type == TREEFILEENTRY_DIRECTORY) {
      cout << "diving into " << entry.filename << endl;
      restore(entry.id, destinationDir);
    }
  }
}
