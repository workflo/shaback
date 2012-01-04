#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <zlib.h>

#include "lib/BufferedWriter.h"
#include "lib/BufferedReader.h"
#include "lib/Date.h"
#include "lib/Exception.h"
#include "lib/FileInputStream.h"
#include "lib/FileOutputStream.h"
#include "lib/Properties.h"
#include "lib/Sha1.h"

#include "Repository.h"
#include "BackupRun.h"
#include "ShabackOutputStream.h"

using namespace std;

Repository::Repository(RuntimeConfig& config) :
  config(config), cache(config.localCacheFile)
{
  Properties props;

  props.load(config.repoPropertiesFile);

  string hashAlgorithm = props.getProperty("digest");
  string compressionAlgorithm = props.getProperty("compression");
  string encryptionAlgorithm = props.getProperty("encryption");

  if (compressionAlgorithm == "Deflate") {
    this->compressionAlgorithm = COMPRESSION_DEFLATE;
  } else if (compressionAlgorithm == "None" || compressionAlgorithm.empty()) {
    this->compressionAlgorithm = COMPRESSION_NONE;
  } else {
    throw UnsupportedCompressionAlgorithm(compressionAlgorithm);
  }

  if (encryptionAlgorithm == "Blowfish") {
    this->encryptionAlgorithm = ENCRYPTION_BLOWFISH;
  } else if (encryptionAlgorithm == "AES") {
    this->encryptionAlgorithm = ENCRYPTION_AES;
  } else if (encryptionAlgorithm == "DES") {
    this->encryptionAlgorithm = ENCRYPTION_DES;
  } else if (encryptionAlgorithm == "None" || encryptionAlgorithm.empty()) {
    this->encryptionAlgorithm = ENCRYPTION_NONE;
  } else {
    throw UnsupportedEncryptionAlgorithm(encryptionAlgorithm);
  }

  if (this->encryptionAlgorithm != ENCRYPTION_NONE) {
    if (config.cryptoPassword.empty())
      throw MissingCryptoPassword();
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
  if (!config.filesDir.isDir() || !config.indexDir.isDir() || !config.locksDir.isDir() || !config.cacheDir.isDir()
      || !config.repoPropertiesFile.isFile()) {
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
  string filename = startDate.toFilename();
  filename.append(".scache");

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

void Repository::storeRootTreeFile(string& rootHashValue)
{
  cout << "rootFile: " << rootHashValue << endl;
  string filename = config.backupName;
  filename.append("_").append(startDate.toFilename()).append(".sroot");

  cout << "Index file: " << filename << endl;

  File file(config.indexDir, filename);
  FileOutputStream os(file);

  os.write(rootHashValue.data(), rootHashValue.size());
}
