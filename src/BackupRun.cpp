#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fnmatch.h>

#include "BackupRun.h"
#include "Sha1.h"

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
    } else {
      // Ignore other types of files.
    }
  }

  string rootFileHashValue = repository.storeTreeFile(rootFile);

  cout << "rootFile: " << rootFileHashValue << endl << rootFile << endl;
}


string BackupRun::handleDirectory(File& dir, bool absolutePaths)
{
  // TODO: Excludes checken

  // TODO: check for oneFileSystem

  string treeFile;
  
  DIR* dirp = opendir(dir.path.c_str());
  struct dirent* dp;

  // TODO: Verzeichnis nach Inodes sortieren

  if (dirp) {
    while ((dp = readdir(dirp)) != NULL) {
      if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;
      
      File child(dir, dp->d_name);
      
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
      closedir(dirp);
  }

  string treeFileHashValue = repository.storeTreeFile(treeFile);
  
  cout << "treeFile: " << dir.path << endl << treeFile << endl;

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
	  dir.statBuffer.st_mode, 
	  dir.statBuffer.st_uid,
	  dir.statBuffer.st_gid,
	  (int)dir.statBuffer.st_mtime,
	  (int)dir.statBuffer.st_ctime);
  treeFileLine.append(buf);
  treeFileLine.append("\n");

  return treeFileLine;
}


string BackupRun::handleFile(File& file, bool absolutePaths)
{
  // TODO: Excludes checken

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
	  file.statBuffer.st_mode, 
	  file.statBuffer.st_uid,
	  file.statBuffer.st_gid,
	  (int)file.statBuffer.st_mtime,
	  (int)file.statBuffer.st_ctime);
  treeFileLine.append(buf);
  treeFileLine.append("\n");

  return treeFileLine;
}


string BackupRun::handleSymlink(File& file, bool absolutePaths)
{
  // TODO: Excludes checken

  string treeFileLine("S\t*\t");
  if (absolutePaths) {
    treeFileLine.append(file.path);
  } else {
    treeFileLine.append(file.fname);
  }

  char buf[100];
  sprintf(buf, "\t%03o\t%d\t%d\t%d\t%d\t\t", 
	  file.statBuffer.st_mode, 
	  file.statBuffer.st_uid,
	  file.statBuffer.st_gid,
	  (int)file.statBuffer.st_mtime,
	  (int)file.statBuffer.st_ctime);
  treeFileLine.append(buf);
  treeFileLine.append(file.readlink());
  treeFileLine.append("\n");

  return treeFileLine;
}
