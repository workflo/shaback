#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include <zlib.h>

#include "Repository.h"
#include "BackupRun.h"
#include "Sha1.h"

using namespace std;

Repository::Repository(RuntimeConfig& config)
  :config(config)
{
  hashAlgorithm = "SHA1";
  cypherAlgorithm = "";
  compressionAlgorithm = "GZ";
}


void Repository::open()
{
  if (!config.filesDir.isDir() || !config.indexDir.isDir() || !config.locksDir.isDir()) {
    cerr << "Does not look like a shaback repository: " << config.repository << endl;
    exit(4);
  }
  
  // TODO: Repo-Konfig auslesen
}


void Repository::lock()
{
  //cout << "lock" << endl;
  // TBI
}


void Repository::unlock()
{
  //cout << "unlock" << endl;
  // TBI	
}


int Repository::backup()
{
  open();
  BackupRun run(config, *this);
  return run.run();
}


File Repository::hashValueToFile(string hashValue)
{
  string path(config.filesDir.path);

  path.append("/").append(hashValue.substr(0, 2));
  path.append("/").append(hashValue.substr(2, 2));
  path.append("/").append(hashValue.substr(4));

  return File(path);
}


bool Repository::contains(File& file)
{
  // TODO: Cache fragen!
  return file.isFile();
}


string Repository::storeTreeFile(string& treeFile)
{
  Sha1 sha1;
  sha1.update(treeFile);
  sha1.finalize();

  File file = hashValueToFile(sha1.toString());
  if (!contains(file)) {
    if (config.verbose) {
      cout << "T: " << file.path << endl;
    }

    // TODO: Erst in .tmp-File schreiben und dann umbenennen
//     int fd = ::open(file.path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0777);
//     if (fd == 0) {
//       // TODO: Fehler
//     }
//     write(fd, treeFile.data(), treeFile.size());
//     close(fd);

    // TODO: KLasse fuer Output-Abstraktion

    gzFile fd = ::gzopen(file.path.c_str(), "wb");
    if (fd == 0) {
      // TODO: Fehler
    }
    gzwrite(fd, treeFile.data(), treeFile.size());
    gzclose(fd);
  }

  return sha1.toString();
}


string Repository::storeFile(File& srcFile)
{
  string hashValue = srcFile.getHashValue();
  File destFile = hashValueToFile(hashValue);

  if (!contains(destFile)) {
    if (config.verbose) {
      cout << "F: " << destFile.path << endl;
    }

    // TODO: Erst in .tmp-File schreiben und dann umbenennen
    srcFile.copyTo(destFile);
  }

  return hashValue;
}

