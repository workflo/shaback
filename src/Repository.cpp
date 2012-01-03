#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

#include <zlib.h>

#include "Repository.h"
#include "BackupRun.h"
#include "Sha1.h"
#include "Exception.h"
#include "FileInputStream.h"
#include "FileOutputStream.h"
#include "BufferedWriter.h"
#include "BufferedReader.h"
#include "ShabackOutputStream.h"

using namespace std;

Repository::Repository(RuntimeConfig& config) :
  config(config), cache(config.localCacheFile)
{
  string hashAlgorithm("SHA1");
  string encryptionAlgorithm("AES");
  string compressionAlgorithm("");

  if (compressionAlgorithm == "Deflate") {
    this->compressionAlgorithm = COMPRESSION_DEFLATE;
  } else if (compressionAlgorithm.empty()) {
    this->compressionAlgorithm = COMPRESSION_NONE;
  } else {
    throw UnsupportedCompressionAlgorithm(compressionAlgorithm);
  }

  if (encryptionAlgorithm == "AES") {
    this->encryptionAlgorithm = ENCRYPTION_AES;
  } else if (encryptionAlgorithm == "DES") {
    this->encryptionAlgorithm = ENCRYPTION_DES;
  } else if (encryptionAlgorithm.empty()) {
    this->encryptionAlgorithm = ENCRYPTION_NONE;
  } else {
    throw UnsupportedEncryptionAlgorithm(encryptionAlgorithm);
  }

  if (this->encryptionAlgorithm != ENCRYPTION_NONE) {
    if (config.cryptoPassword.empty()) throw MissingCryptoPassword();
  }

  if (!config.localCacheFile.empty()) {
    cache.open(GDBM_NEWDB);
    importCacheFile();
  }
}

Repository::~Repository()
{
  if (!config.localCacheFile.empty()) {
    exportCacheFile();
    cache.close();
  }
}

void Repository::open()
{
  if (!config.filesDir.isDir() || !config.indexDir.isDir()
      || !config.locksDir.isDir() || !config.cacheDir.isDir()) {
    cerr << "Does not look like a shaback repository: " << config.repository
        << endl;
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
  int rc = run.run();

  if (config.showTotals) {
    run.showTotals();
  }

  return rc;
}

File Repository::hashValueToFile(string hashValue)
{
  string path(config.filesDir.path);

  path.append("/").append(hashValue.substr(0, 2));
  path.append("/").append(hashValue.substr(2, 2));
  path.append("/").append(hashValue.substr(4));

  return File(path);
}

bool Repository::contains(string& hashValue)
{
  return cache.contains(hashValue) || hashValueToFile(hashValue).isFile();
}

string Repository::storeTreeFile(BackupRun* run, string& treeFile)
{
  Sha1 sha1;
  sha1.update(treeFile);
  sha1.finalize();
  string hashValue = sha1.toString();

  if (!contains(hashValue)) {
    File file = hashValueToFile(hashValue);

    if (config.debug) {
      cout << "[t] " << file.path << endl;
    }

    ShabackOutputStream os(config, compressionAlgorithm, encryptionAlgorithm);
    os.open(file);
    os.write(treeFile);

    run->numBytesStored += treeFile.size();
  }

  cache.put(hashValue);

  return sha1.toString();
}

#define READ_BUFFER_SIZE (1024 * 4)
static char readBuffer[READ_BUFFER_SIZE];

string Repository::storeFile(BackupRun* run, File& srcFile)
{
  run->numFilesRead++;
  run->numBytesRead += srcFile.getSize();

  Sha1 sha1;
  FileInputStream in(srcFile);
  while (true) {
    int bytesRead = in.read(readBuffer, READ_BUFFER_SIZE);
    if (bytesRead == -1)
      break;
    sha1.update(readBuffer, bytesRead);
  }

  sha1.finalize();
  string hashValue = sha1.toString();

  if (!contains(hashValue)) {
    in.reset();

    File destFile = hashValueToFile(hashValue);

    if (config.verbose || config.debug) {
      cout << "[m] " << srcFile.path << endl;
      if (config.debug) {
        cout << "[f] " << destFile.path << endl;
      }
    }

    ShabackOutputStream os(config, compressionAlgorithm, encryptionAlgorithm);
    os.open(destFile);

    while (true) {
      int bytesRead = in.read(readBuffer, READ_BUFFER_SIZE);
      if (bytesRead == -1)
        break;
      os.write(readBuffer, bytesRead);
    }

    run->numFilesStored++;
    run->numBytesStored += srcFile.getSize();
  } else {
    if (config.debug) {
      cout << "[ ] " << srcFile.path << endl;
    }
  }

  cache.put(hashValue);

  return hashValue;
}

void Repository::exportCacheFile()
{
  time_t rawtime;
  struct tm * ptm;
  time(&rawtime);
  ptm = gmtime(&rawtime);

  char filename[100];
  sprintf(filename, "%04d-%02d-%02d_%02d%02d%02d.scache", ptm->tm_year + 1900,
      ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

  File file(config.cacheDir, filename);
  FileOutputStream os(file);
  BufferedWriter writer(&os);
  cache.exportCache(writer);
}

void Repository::importCacheFile()
{
  vector<File> files = config.cacheDir.listFiles("*.scache");

  if (!files.empty()) {
    File& file = files.back();
    if (config.verbose)
      cout << "Preloading cache from: " << file.path << endl;
    FileInputStream is(file);
    BufferedReader reader(&is);
    int count = cache.importCache(reader);
    if (config.verbose)
      cout << "Cache contains " << count << " entries." << endl;
  }
}
