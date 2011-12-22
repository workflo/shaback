#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include <zlib.h>

#include "Repository.h"
#include "BackupRun.h"
#include "Sha1.h"
#include "Exception.h"
#include "OutputStream.h"

using namespace std;

Repository::Repository(RuntimeConfig& config)
  :config(config), cache(config.localCacheFile)
{
  string hashAlgorithm("SHA1");
  string encryptionAlgorithm("");
  string compressionAlgorithm("GZ");

  if (compressionAlgorithm == "GZ") {
    this->compressionAlgorithm = COMPRESSION_GZ;
  } else if (compressionAlgorithm == "LZO") {
    this->compressionAlgorithm = COMPRESSION_LZO;
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
  
  if (!config.localCacheFile.empty()) {
    cache.open();
  }
}

Repository::~Repository()
{
  cache.close();
}


void Repository::open()
{
  if (!config.filesDir.isDir() || !config.indexDir.isDir() || !config.locksDir.isDir() || !config.cacheDir.isDir()) {
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


bool Repository::contains(string& hashValue)
{
  return cache.contains(hashValue) || hashValueToFile(hashValue).isFile();
}


string Repository::storeTreeFile(string& treeFile)
{
  Sha1 sha1;
  sha1.update(treeFile);
  sha1.finalize();
  string hashValue = sha1.toString();

  if (!contains(hashValue)) {
    File file = hashValueToFile(hashValue);
    
    if (config.verbose) {
      cout << "T: " << file.path << endl;
    }

    // TODO: Erst in .tmp-File schreiben und dann umbenennen

    OutputStream os(compressionAlgorithm, encryptionAlgorithm);
    os.open(file);
    os.write(treeFile);

    cache.put(hashValue);
  }

  return sha1.toString();
}


#define READ_BUFFER_SIZE (1024 * 4)
static char readBuffer[READ_BUFFER_SIZE];

string Repository::storeFile(File& srcFile)
{
  string hashValue = srcFile.getHashValue();

  if (!contains(hashValue)) {
    File destFile = hashValueToFile(hashValue);
    
    if (config.verbose) {
      cout << "F: " << destFile.path << endl;
    }

    // TODO: Erst in .tmp-File schreiben und dann umbenennen

    OutputStream os(compressionAlgorithm, encryptionAlgorithm);
    os.open(destFile);

    int fdIn = ::open(srcFile.path.c_str(), O_RDONLY);
    if (fdIn == 0) {
      throw Exception::errnoToException(srcFile.path);
    }
    
    while (true) {
      ssize_t bytesRead = read(fdIn, readBuffer, READ_BUFFER_SIZE);
      if (bytesRead == -1) {
	throw Exception::errnoToException(srcFile.path);
      } else if (bytesRead == 0) {
	break;
      }
      os.write(readBuffer, bytesRead);
    }

    close(fdIn);

    cache.put(hashValue);
  }

  return hashValue;
}
