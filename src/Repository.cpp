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
#include <fcntl.h>
#include <time.h>
#include <zlib.h>

#include "lib/BufferedWriter.h"
#include "lib/BufferedReader.h"
#include "lib/Date.h"
#include "lib/FileInputStream.h"
#include "lib/FileOutputStream.h"
#include "lib/StandardOutputStream.h"
#include "lib/Properties.h"
#include "lib/Sha1.h"
#include "lib/Sha256.h"

#include "BackupRun.h"
#include "GarbageCollection.h"
#include "Repository.h"
#include "RestoreGui.h"
#include "RestoreRun.h"
#include "ShabackInputStream.h"
#include "ShabackOutputStream.h"
#include "ShabackException.h"
#include "TreeFile.h"

using namespace std;

Repository::Repository(RuntimeConfig& config) :
  config(config), cache(config.localCacheFile)
{
}

Repository::~Repository()
{
  if (!config.localCacheFile.empty()) {
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

  Properties props;

  props.load(config.repoPropertiesFile);

  string hashAlgorithm = props.getProperty("digest");

  compressionAlgorithm = compressionByName(props.getProperty("compression"));
  encryptionAlgorithm = encryptionByName(props.getProperty("encryption"));

  if (encryptionAlgorithm != ENCRYPTION_NONE) {
    if (config.cryptoPassword.empty())
      throw MissingCryptoPassword();
    checkPassword();
  }
}

void Repository::checkPassword()
{
  FileInputStream in(config.passwordCheckFile);
  string hashFromFile;
  string hashFromPassword = hashPassword(config.cryptoPassword);

  if (in.readAll(hashFromFile)) {
    if (hashFromFile == hashFromPassword) {
      // Passwords match!
    } else {
      throw PasswordException(string("Wrong password provided for repository ").append(config.repoDir.path));
    }
  } else {
    throw PasswordException(string("Password file does not contain hashed password: ").append(config.passwordCheckFile.path));
  }
}

void Repository::lock(bool exclusive)
{
  int lockFileFh = ::open(config.lockFile.path.c_str(), O_CREAT | O_EXCL, S_IRWXU | S_IROTH | S_IRGRP );
  if (lockFileFh == -1)
    throw Exception::errnoToException(config.lockFile.path);
  
  int ret = ::symlink(config.lockFile.fname.c_str(), config.exclusiveLockFile.path.c_str());
  if (ret != 0) {
    if (errno == EEXIST) {
      throw LockingException(string("Repository is locked exclusively. Check lock files in ").append(config.locksDir.path));
      // TODO: EPERM: symlinks not supported!
    } else {
      throw Exception::errnoToException(config.exclusiveLockFile.path);
    }
  }

  if (exclusive) {
    config.haveExclusiveLock = true;

    // Look for other, non-exclusive locks
    vector<File> lockFiles = config.locksDir.listFiles("*.lock");
    if (lockFiles.size() > 1) {
      throw LockingException(string("Cannot exclusively lock repository while other locks exist. Check lock files in ").append(config.locksDir.path));
    }
  } else {
    config.exclusiveLockFile.remove();
  }
}

void Repository::unlock()
{
  config.lockFile.remove();
  if (config.haveExclusiveLock) {
    config.exclusiveLockFile.remove();
  }
}

void Repository::openCache()
{
  cache.open(GDBM_NEWDB);
}

int Repository::backup()
{
  open();
  if (!config.localCacheFile.empty()) {
    openCache();
    importCacheFile();
  }

  BackupRun run(config, *this);
  int rc = run.run();

  if (config.showTotals) {
    run.showTotals();
  }

  if (!config.localCacheFile.empty()) {
    exportCacheFile();
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
  //  return hashValueToFile(hashValue).isFile();
}

string Repository::storeTreeFile(BackupRun* run, string& treeFile)
{
  Sha1 sha1;
  sha1.update(treeFile);
  sha1.finalize();
  string hashValue = sha1.toString();

  //  cerr << hashValue << ":\n" << treeFile << "\n--------------------" << endl;

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

  string hashValue = srcFile.getXAttr("user.shaback.sha1");

  FileInputStream in(srcFile);

  if (hashValue.empty() || strtol(srcFile.getXAttr("user.shaback.mtime").c_str(), 0, 10) != srcFile.getPosixMtime()) {
    Sha1 sha1;
    while (true) {
      int bytesRead = in.read(readBuffer, READ_BUFFER_SIZE);
      if (bytesRead == -1)
        break;
      sha1.update(readBuffer, bytesRead);
    }

    sha1.finalize();
    hashValue = sha1.toString();

    srcFile.setXAttr("user.shaback.sha1", hashValue); // TODO: Use dynamic digest name
    srcFile.setXAttr("user.shaback.mtime", srcFile.getPosixMtime());
  }

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

vector<TreeFileEntry> Repository::loadTreeFile(string& treeId)
{
  File file = hashValueToFile(treeId);
  ShabackInputStream in(config, compressionAlgorithm, encryptionAlgorithm);
  in.open(file);

  string content;
  in.readAll(content);

  vector<TreeFileEntry> list;
  int from = 0;
  int until;

  if ((until = content.find('\n', from)) == string::npos)
    throw InvalidTreeFile("Missing header line");
  string header = content.substr(from, until - from);
  if (header != TREEFILE_HEADER)
    throw InvalidTreeFile("Unexpected header line in tree file");
  from = until + 1;

  if ((until = content.find('\n', from)) == string::npos)
    throw InvalidTreeFile("Missing parent directory line");
  string parentDir = content.substr(from, until - from);
  from = until + 1;

  while ((until = content.find('\n', from)) != string::npos) {
    string line = content.substr(from, until - from);
    list.push_back(TreeFileEntry(line, parentDir));
    from = until + 1;
  }

  return list;
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
  string filename = config.backupName;
  filename.append("_").append(startDate.toFilename()).append(".sroot");

  File file(config.indexDir, filename);
  FileOutputStream os(file);

  os.write(rootHashValue.data(), rootHashValue.size());

  os.close();

  cout << "ID:         " << rootHashValue << endl;
  cout << "Index file: " << file.path << endl;
}

void Repository::restore()
{
  open();

  if (config.cliArgs.empty()) {
    RestoreGui gui(config, *this);
    gui.run();
  } else {
    string treeSpec = config.cliArgs.at(0);

    if (Digest::looksLikeDigest(treeSpec)) {
      restoreByTreeId(treeSpec);
    } else if (treeSpec.rfind(".sroot") == treeSpec.size() - 6) {
      string fname = treeSpec.substr(treeSpec.rfind(File::separator) + 1);
      File rootFile(config.indexDir, fname);

      restoreByRootFile(rootFile);
    } else {
      throw RestoreException(string("Don't know how to restore `").append(treeSpec).append("'."));
    }
  }
}

void Repository::restoreByRootFile(File& rootFile)
{
  FileInputStream in(rootFile);
  string hashValue;
  if (in.readLine(hashValue)) {
    restoreByTreeId(hashValue);
  } else {
    throw RestoreException(string("Root index file is empty: ").append(rootFile.path));
  }
}

void Repository::restoreByTreeId(string& treeId)
{
  RestoreRun run(config, *this);
  File destinationDir(".");
  run.restore(treeId, destinationDir);

  if (config.showTotals) {
    run.showTotals();
  }
}

void Repository::exportFile(string& id, OutputStream& out)
{
  File inFile = hashValueToFile(id);
  ShabackInputStream in(config, compressionAlgorithm, encryptionAlgorithm);
  in.open(inFile);

  in.copyTo(out);
  out.close();
}

void Repository::exportSymlink(TreeFileEntry& entry, File& linkFile)
{
  linkFile.remove();

  int ret = ::symlink(entry.symLinkDest.c_str(), linkFile.path.c_str());
  if (ret != 0)
    throw Exception::errnoToException(linkFile.path);
}

void Repository::show()
{
  if (config.cliArgs.empty()) {
    throw RestoreException("Don't know what to show.");
  }

  open();

  string id = config.cliArgs.at(0);
  StandardOutputStream out(stdout);
  exportFile(id, out);
}

void Repository::gc()
{
  open();
  GarbageCollection gc(config, *this);
  gc.run();
}

int Repository::encryptionByName(string name)
{
  if (name == "Blowfish") {
    return ENCRYPTION_BLOWFISH;
    //  } else if (name == "Twofish") {
    //    return ENCRYPTION_TWOFISH;
    //  } else if (name == "AES") {
    //    return ENCRYPTION_AES;
    //  } else if (name == "DES") {
    //    return ENCRYPTION_DES;
  } else if (name == "None" || name.empty()) {
    return ENCRYPTION_NONE;
  } else {
    throw UnsupportedEncryptionAlgorithm(name);
  }
}

int Repository::compressionByName(string name)
{
  if (name == "Deflate") {
    return COMPRESSION_DEFLATE;
  } else if (name == "None" || name.empty()) {
    return COMPRESSION_NONE;
  } else {
    throw UnsupportedCompressionAlgorithm(name);
  }
}

string Repository::compressionToName(int compression)
{
  switch(compression) {
    case COMPRESSION_DEFLATE:
      return "Deflate";
    default:
      return "None";
  }
}

string Repository::encryptionToName(int encryption)
{
  switch(encryption) {
    case ENCRYPTION_BLOWFISH:
      return "Blowfish";
    default:
      return "None";
  }
}

string Repository::hashPassword(string password)
{
  Sha256 sha;
  string salt(PASSWORDFILE_SALT);
  sha.update(salt);
  sha.update(password);
  sha.finalize();
  return sha.toString();
}

void Repository::removeAllCacheFiles()
{
  vector<File> cacheFiles = config.cacheDir.listFiles("*.scache");

  for (vector<File>::iterator it = cacheFiles.begin(); it < cacheFiles.end(); it++) {
    File cacheFile(*it);
    cacheFile.remove();
  }
}
