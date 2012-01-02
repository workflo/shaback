#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "BackupRun.h"
#include "Sha1.h"
#include "Exception.h"

using namespace std;

BackupRun::BackupRun(RuntimeConfig& config, Repository& repository)
  :repository(repository), config(config)
{
  repository.lock();
}

BackupRun::~BackupRun()
{
  repository.unlock();
}


int BackupRun::run()
{
  if (config.dirs.empty()) {
    cerr << "No files to backup." << endl;
    return 5;
  }

  string rootFile;
  
  for (vector<string>::iterator it = config.dirs.begin(); it < config.dirs.end(); it++ ) {
    File file(*it);

    if (file.isSymlink()) {
      rootFile.append(handleSymlink(file, true));
    } else if (file.isDir()) {
      rootFile.append(handleDirectory(file, true));
    } else if (file.isFile()) {
      rootFile.append(handleFile(file, true));
    } else if (!file.exists()) {
      throw FileNotFoundException(file.path);
    } else {
      // Ignore other types of files.
    }
  }

  string rootFileHashValue = repository.storeTreeFile(rootFile);

  cout << "rootFile: " << rootFileHashValue << endl;
  if (config.verbose) {
    cout << rootFile << endl;
  }

  return 0;
}


string BackupRun::handleDirectory(File& dir, bool absolutePaths)
{
  // TODO: check for oneFileSystem

  string treeFile;
  
  vector<File> files = dir.listFiles("*");

  // TODO: Verzeichnis nach Inodes sortieren

  for (vector<File>::iterator it = files.begin(); it < files.end(); it++ ) {
    File child(*it);

    if (config.excludeFile(child)) continue;
	
    if (child.isSymlink()) {
	treeFile.append(handleSymlink(child, false));
    } else if (child.isDir()) {
	treeFile.append(handleDirectory(child, false));
    } else if (child.isFile()) {
	treeFile.append(handleFile(child, false));
    } else {
	// Ignore other types of files.
    }
  }

  string treeFileHashValue = repository.storeTreeFile(treeFile);
  
//   cout << "treeFile: " << dir.path << endl << treeFile << endl;

  string treeFileLine("D\t");
  treeFileLine.append(treeFileHashValue);
  treeFileLine.append("\t");
  if (absolutePaths) {
    treeFileLine.append(dir.path);
  } else {
    treeFileLine.append(dir.fname);
  }

  char buf[100];
  sprintf(buf, "\t%03o\t%d\t%d\t%d\t%d\t\t", 
	  dir.getPosixMode(), 
	  dir.getPosixUid(),
	  dir.getPosixGid(),
	  dir.getPosixMtime(),
	  dir.getPosixCtime());
  treeFileLine.append(buf);
  treeFileLine.append("\n");

  return treeFileLine;
}


string BackupRun::handleFile(File& file, bool absolutePaths)
{
  string hashValue = repository.storeFile(file);
  
  string treeFileLine("F\t");
  treeFileLine.append(hashValue);
  treeFileLine.append("\t");
  if (absolutePaths) {
    treeFileLine.append(file.path);
  } else {
    treeFileLine.append(file.fname);
  }

  char buf[100];
  sprintf(buf, "\t%03o\t%d\t%d\t%d\t%d\t\t", 
	  file.getPosixMode(), 
	  file.getPosixUid(),
	  file.getPosixGid(),
	  file.getPosixMtime(),
	  file.getPosixCtime());
  treeFileLine.append(buf);
  treeFileLine.append("\n");

  return treeFileLine;
}


string BackupRun::handleSymlink(File& file, bool absolutePaths)
{
  string treeFileLine("S\t*\t");
  if (absolutePaths) {
    treeFileLine.append(file.path);
  } else {
    treeFileLine.append(file.fname);
  }

  char buf[100];
  sprintf(buf, "\t%03o\t%d\t%d\t%d\t%d\t\t", 
	  file.getPosixMode(), 
	  file.getPosixUid(),
	  file.getPosixGid(),
	  file.getPosixMtime(),
	  file.getPosixCtime());
  treeFileLine.append(buf);
  treeFileLine.append(file.readlink());
  treeFileLine.append("\n");

  return treeFileLine;
}
