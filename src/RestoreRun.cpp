#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "RestoreRun.h"
#include "lib/Exception.h"

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

}
