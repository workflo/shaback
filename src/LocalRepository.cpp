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
#include <algorithm>
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

#include "ShabackConfig.h"
#include "BackupRun.h"
#include "GarbageCollection.h"
#include "LocalRepository.h"
#include "RestoreRun.h"
#include "ShabackInputStream.h"
#include "ShabackOutputStream.h"
#include "ShabackException.h"
#include "SplitFileIndexReader.h"
#include "TreeFile.h"

#define READ_BUFFER_SIZE (1024 * 4)

using namespace std;

LocalRepository::LocalRepository(RuntimeConfig& config) :
    Repository(config), readCache(config.readCacheFile)
{
//  splitBlockSize = 1024 * 1024 * 5;
//  splitMinBlocks = 5;

//  readBuffer = (char*) malloc(max(READ_BUFFER_SIZE, splitBlockSize));
}

LocalRepository::~LocalRepository()
{
  readCache.close();
//  free(readBuffer);
}

void LocalRepository::open()
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
  repoFormat = repoFormatByName(props.getProperty("repoFormat"));

  if (encryptionAlgorithm != ENCRYPTION_NONE) {
    if (config.cryptoPassword.empty())
      throw MissingCryptoPassword();
    checkPassword();
  }
}

void LocalRepository::checkPassword()
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
    throw PasswordException(
        string("Password file does not contain hashed password: ").append(config.passwordCheckFile.path));
  }
}

void LocalRepository::lock(bool exclusive)
{
  int lockFileFh = ::open(config.lockFile.path.c_str(), O_CREAT | O_EXCL, S_IRWXU | S_IROTH | S_IRGRP);
  if (lockFileFh == -1)
    throw Exception::errnoToException(config.lockFile.path);

  int ret = ::symlink(config.lockFile.fname.c_str(), config.exclusiveLockFile.path.c_str());
  if (ret != 0) {
    if (errno == EEXIST) {
      throw LockingException(
          string("Repository is locked exclusively. Check lock files in ").append(config.locksDir.path));
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
      throw LockingException(
          string("Cannot exclusively lock repository while other locks exist. Check lock files in ").append(
              config.locksDir.path));
    }
  } else {
    config.exclusiveLockFile.remove();
  }
}

void LocalRepository::unlock()
{
  config.lockFile.remove();
  if (config.haveExclusiveLock) {
    config.exclusiveLockFile.remove();
  }
}

void LocalRepository::openReadCache()
{
  try {
    readCache.open(GDBM_WRCREAT);
  } catch (Exception &ex) {
    cerr << "Warning: Unable to open read cache file: " << ex.getMessage() << endl;
  }
}

int LocalRepository::backup()
{
  open();
  if (config.useWriteCache) importCacheFile();

  BackupRun run(config, *this);
  int rc = run.run();

  if (config.showTotals) {
    run.showTotals();
  }

  if (config.useWriteCache) exportCacheFile();

  config.runPostBackupCallbacks(&run);

  return rc;
}

File LocalRepository::hashValueToFile(string hashValue)
{
  string path(config.filesDir.path);

  switch (repoFormat) {
    case REPOFORMAT_3:
      path.append("/").append(hashValue.substr(0, 3));
      path.append("/").append(hashValue.substr(3));
      break;
    default:
      path.append("/").append(hashValue.substr(0, 2));
      path.append("/").append(hashValue.substr(2, 2));
      path.append("/").append(hashValue.substr(4));
      break;
  }

  return File(path);
}

bool LocalRepository::contains(string& hashValue)
{
  return writeCache.count(hashValue) || hashValueToFile(hashValue).isFile();
}

void LocalRepository::storeTextFile(string hashValue, string content)
{
  File file = hashValueToFile(hashValue);

  ShabackOutputStream os = createOutputStream();
  os.open(file);
  os.write(content);
  os.finish();
}

//string LocalRepository::storeTreeFile(BackupRun* run, string& treeFile)
//{
//  Sha1 sha1;
//  sha1.update(treeFile);
//  sha1.finalize();
//  string hashValue = sha1.toString();
//
//  if (!contains(hashValue)) {
//    File file = hashValueToFile(hashValue);
//
//    if (config.debug) {
//      cout << "[t] " << file.path << endl;
//    }
//
//    ShabackOutputStream os = createOutputStream();
//    os.open(file);
//    os.write(treeFile);
//    os.finish();
//
//    run->numBytesStored += treeFile.size();
//  }
//
//  writeCache.insert(hashValue);
//
//  return hashValue;
//}

void LocalRepository::store(BackupRun* run, File& srcFile, InputStream& in, string& hashValue)
{
  const bool split = config.splitFile(srcFile);

  in.reset();

  File destFile = hashValueToFile(hashValue);

  if (config.verbose || config.debug) {
    cout << (split ? "[s] " : "[m] ") << srcFile.path << endl;
    if (config.debug) {
      cout << "[f] " << destFile.path << endl;
    }
  }

  ShabackOutputStream os = createOutputStream();
  os.open(destFile);

  if (split) {
    storeSplitFile(run, hashValue, in, os);
  } else {
    while (true) {
      int bytesRead = in.read(readBuffer, READ_BUFFER_SIZE);
      if (bytesRead == -1)
        break;
      os.write(readBuffer, bytesRead);
      run->numBytesStored += bytesRead;
    }
  }

  os.finish();

  run->numFilesStored++;
}

void LocalRepository::storeSplitFile(BackupRun* run, string& fileHashValue, InputStream &in,
    ShabackOutputStream &blockFileOut)
{
  Sha1 totalSha1;
  int blockCount = 0;

  while (true) {
    Sha1 blockSha1;
    const int bytesRead = in.read(readBuffer, splitBlockSize);
    if (bytesRead == -1)
      break;

    blockSha1.update(readBuffer, bytesRead);
    blockSha1.finalize();
    string blockHashValue = blockSha1.toString();

    blockCount++;

    if (config.verbose) {
      printf("  Storing block: %8d", blockCount);
      cout << "\r";
      cout << flush;
    }

    if (!contains(blockHashValue)) {
      File blockDestFile = hashValueToFile(blockHashValue);

      ShabackOutputStream os = createOutputStream();
      os.open(blockDestFile);

      os.write(readBuffer, bytesRead);
      os.finish();

      writeCache.insert(blockHashValue);
      run->numBytesStored += bytesRead;
    }

    blockFileOut.write(blockHashValue);
    blockFileOut.write("\n");

    totalSha1.update(readBuffer, bytesRead);
  }

  if (config.verbose)
    cout << endl;

  totalSha1.finalize();
  string totalHashValue = totalSha1.toString();

  if (fileHashValue.compare(0, totalHashValue.size(), totalHashValue) != 0) {
    // TODO: throw exception if hash value has changed
//    cerr << "File changed while being backed up: " << fileSha1.toString() << " <> " << fileHashValue << endl;
  }
}

vector<TreeFileEntry> LocalRepository::loadTreeFile(string& treeId)
{
  string content;
  bool fromCache;

  if (readCache.contains(treeId)) {
    content = readCache.get(treeId);
    fromCache = true;
  } else {
    File file = hashValueToFile(treeId);
    ShabackInputStream in = createInputStream();
    in.open(file);

    in.readAll(content);
    fromCache = false;
  }

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

  if (!fromCache) {
    readCache.put(treeId, content);
  }

  return list;
}

void LocalRepository::exportCacheFile()
{
  string filename = startDate.toFilename();
  filename.append(".scache");

  File file(config.cacheDir, filename);
  FileOutputStream os(file);
  BufferedWriter writer(&os);

  set<string>::iterator it;

  for (it = writeCache.begin(); it != writeCache.end(); it++) {
    string s(*it);
    writer.write(s);
    writer.newLine();
  }
}

void LocalRepository::importCacheFile()
{
  vector<File> files = config.cacheDir.listFiles("*.scache");

  sort(files.begin(), files.end(), filePathComparator);

  if (!files.empty()) {
    File& file = files.back();
    if (config.verbose)
      cout << "Preloading cache from: " << file.path << endl;

    FileInputStream is(file);
    BufferedReader reader(&is);
    int count = 0;
    string str;

    while (reader.readLine(str)) {
      writeCache.insert(str);
      count++;
    }

    if (config.verbose)
      cout << "Cache contains " << count << " entries." << endl;
  }
}

void LocalRepository::storeRootTreeFile(string rootHashValue, string filename)
{
  File file(config.indexDir, filename);
  FileOutputStream os(file);

  os.write(rootHashValue.data(), rootHashValue.size());

  os.close();
}

//void LocalRepository::storeRootTreeFile(string& rootHashValue)
//{
//  string filename = config.backupName;
//  filename.append("_").append(startDate.toFilename()).append(".sroot");
//
//  File file(config.indexDir, filename);
//  FileOutputStream os(file);
//
//  os.write(rootHashValue.data(), rootHashValue.size());
//
//  os.close();
//
//  cout << "ID:         " << rootHashValue << endl;
//  cout << "Index file: " << file.path << endl;
//}

int LocalRepository::restore()
{
  if (config.cliArgs.empty()) {
    throw RestoreException("Don't know what to restore.");
  }

  open();
  // openReadCache();

  string treeSpec = config.cliArgs.at(0);

  if (Digest::looksLikeDigest(treeSpec)) {
    return restoreByTreeId(treeSpec);
  } else if (treeSpec.rfind(".sroot") == treeSpec.size() - 6) {
    string fname = treeSpec.substr(treeSpec.rfind(File::separator) + 1);
    File rootFile(config.indexDir, fname);

    return restoreByRootFile(rootFile);
  } else {
    throw RestoreException(string("Don't know how to restore `").append(treeSpec).append("'."));
  }
}

int LocalRepository::restoreByRootFile(File& rootFile)
{
  FileInputStream in(rootFile);
  string hashValue;
  if (in.readLine(hashValue)) {
    return restoreByTreeId(hashValue);
  } else {
    throw RestoreException(string("Root index file is empty: ").append(rootFile.path));
  }
}

int LocalRepository::restoreByTreeId(string& treeId)
{
  RestoreRun run(config, *this);
  File destinationDir(".");
  run.restore(treeId, destinationDir);

  if (config.showTotals) {
    run.showTotals();
  }

  return (run.numErrors > 0 ? 1 : 0);
}

void LocalRepository::exportFile(TreeFileEntry& entry, OutputStream& out)
{
  if (entry.isSplitFile) {
    SplitFileIndexReader reader(*this, entry.id);
    string hashValue;
    while (reader.next(hashValue)) {
      ShabackInputStream in = createInputStream();
      File blockFile = hashValueToFile(hashValue);
      in.open(blockFile);
      in.copyTo(out);
    }
    out.close();
  } else {
    exportFile(entry.id, out);
  }
}

void LocalRepository::exportFile(string& id, OutputStream& out)
{
  File inFile = hashValueToFile(id);
  ShabackInputStream in = createInputStream();
  in.open(inFile);

  in.copyTo(out);
  out.close();
}

void LocalRepository::exportSymlink(TreeFileEntry& entry, File& linkFile)
{
  linkFile.remove();

  int ret = ::symlink(entry.symLinkDest.c_str(), linkFile.path.c_str());
  if (ret != 0)
    throw Exception::errnoToException(linkFile.path);
}

void LocalRepository::show()
{
  if (config.cliArgs.empty()) {
    throw RestoreException("Don't know what to show.");
  }

  open();

  string id = config.cliArgs.at(0);
  StandardOutputStream out(stdout);
  exportFile(id, out);
}

void LocalRepository::gc()
{
  open();
  GarbageCollection gc(config, *this);
  gc.run();
}

void LocalRepository::removeAllCacheFiles()
{
  vector<File> cacheFiles = config.cacheDir.listFiles("*.scache");

  for (vector<File>::iterator it = cacheFiles.begin(); it < cacheFiles.end(); it++) {
    File cacheFile(*it);
    cacheFile.remove();
  }
}

ShabackInputStream LocalRepository::createInputStream()
{
  return ShabackInputStream(config, compressionAlgorithm, encryptionAlgorithm);
}

ShabackOutputStream LocalRepository::createOutputStream()
{
  return ShabackOutputStream(config, compressionAlgorithm, encryptionAlgorithm);
}

void LocalRepository::deleteOldIndexFiles()
{
  string pattern(config.backupName);
  pattern.append("_????" "-??" "-??_??????.sroot");

  vector<File> indexFiles = config.indexDir.listFiles(pattern);

  if (indexFiles.size() <= 2) return; // Don't delete latest two backups

  sort(indexFiles.begin(), indexFiles.end(), filePathComparator);
  reverse(indexFiles.begin(), indexFiles.end());
  vector<Date> dates;
  vector<Date> toDelete;

  for (vector<File>::iterator it = indexFiles.begin(); it < indexFiles.end(); it++) {
    File file(*it);
    Date d(file.fname.substr(config.backupName.size() + 1));
    dates.push_back(d);
  }

  int idx = 0;
  Date now;

  Date upper(now);
  upper.addDays(-config.keepOldBackupsBoundaries[0]);
  upper.setTimeOfDay(0, 0, 0);

//  cout << "   upper: " << upper.toFilename() << endl;

  // Keep daily backups:
  Date dailyLimit(upper);
  dailyLimit.addDays(-config.keepOldBackupsBoundaries[1]);
  while (true) {
    Date lower(upper);
    lower.addDays(-1);
    if (lower.compareTo(dailyLimit) <= 0)
      break;
    int n = 0;
//    cout << "   deleting " << lower.toFilename() << " .. " << upper.toFilename() << endl;
    for (vector<Date>::iterator it = dates.begin(); it < dates.end(); it++) {
      Date d(*it);
      if (d.compareTo(lower) >= 0 && d.compareTo(upper) < 0) {
        n++;
        if (n > 1) {
          toDelete.push_back(d);
        }
      }
    }
    upper = lower;
  }

  // Keep weekly backups:
  Date weeklyLimit(upper);
  weeklyLimit.addDays(-config.keepOldBackupsBoundaries[2]);
  while (true) {
    Date lower(upper);
    lower.addDays(-7);
    if (lower.compareTo(weeklyLimit) <= 0)
      break;
    int n = 0;
//    cout << "   deleting " << lower.toFilename() << " .. " << upper.toFilename() << endl;
    for (vector<Date>::iterator it = dates.begin(); it < dates.end(); it++) {
      Date d(*it);
      if (d.compareTo(lower) >= 0 && d.compareTo(upper) < 0) {
        n++;
        if (n > 1) {
          toDelete.push_back(d);
        }
      }
    }
    upper = lower;
  }

  // Keep monthly backups:
  Date monthlyLimit(dates.back());
  while (true) {
    Date lower(upper);
    lower.addDays(-30);
    int n = 0;
//    cout << "   deleting " << lower.toFilename() << " .. " << upper.toFilename() << endl;
    for (vector<Date>::iterator it = dates.begin(); it < dates.end(); it++) {
      Date d(*it);
      if (d.compareTo(lower) >= 0 && d.compareTo(upper) < 0) {
        n++;
        if (n > 1) {
          toDelete.push_back(d);
        }
      }
    }
    upper = lower;
    if (lower.compareTo(monthlyLimit) <= 0)
      break;
  }

  // Actually delete files:
  for (vector<Date>::iterator it = toDelete.begin(); it < toDelete.end(); it++) {
    Date d(*it);
    string fname(config.backupName);
    fname.append("_").append(d.toFilename()).append(".sroot");
    File file(config.indexDir, fname);
    if (config.verbose) {
      cout << "Deleting old index file " << file.path.c_str() << endl;
    }
    file.remove();
  }
}

int LocalRepository::remoteCommandListener()
{
  try {
    open();
  } catch (Exception &ex) {
    cout << "ERROR " << ex.getMessage() << "\n";
    return 1;
  }

  cout << "SHABACK " << SHABACK_VERSION_MAJOR << "." << SHABACK_VERSION_MINOR << "\n";

  string cmdline;

  for (;;) {
    cout.flush();
    std::getline (cin, cmdline);

    if (cin.eof()) return 0;
    if (cin.fail()) return 2;

    string cmd;
    string args;

    int sepPos = cmdline.find(" ");
    if (sepPos > 0) {
      cmd = cmdline.substr(0, sepPos);
      args = cmdline.substr(sepPos +1);
    } else {
      cmd = cmdline;
    }

    try {
      if (cmd == "lock") {
        lock();
        cout << "OK\n";
      }

      else if (cmd == "unlock") {
        unlock();
        cout << "OK\n";
      }

      else if (cmd == "contains") {
        bool c = contains(args);
        cout << "OK " << (c ? "1" : "0") << "\n";
      }

      else if (cmd == "storeTextFile") {
        remoteStoreTextFile(cmdline, args);
      }

      else if (cmd == "storeRootTreeFile") {
        remoteStoreRootTreeFile(cmdline, args);
      }

      else if (cmd == "close") {
        return 0;
      }

      else {
        cout << "ERROR Invalid remote command: " << cmdline << "\n";
      }
    } catch (Exception &ex) {
      cout << "ERROR " << ex.getMessage() << "\n";
    }
  }
}

void LocalRepository::remoteStoreTextFile(string& cmdline, string& args)
{
  int pos = args.find(" ");
  if (pos > 0) {
    string hashValue = args.substr(0, pos);
    long length = atol(args.substr(pos +1).c_str());

    File file = hashValueToFile(hashValue);
    cout << "OK ready to receive " << file.path << "\n";

    ShabackOutputStream os = createOutputStream();
    os.open(file);
    os.write(cin, length);
    os.finish();

    cout << "OK\n";
  } else {
    cout << "ERROR syntax error: " << cmdline << "\n";
  }
}

void LocalRepository::remoteStoreRootTreeFile(string& cmdline, string& args)
{
  int pos = args.find(" ");
  if (pos > 0) {
    string hashValue = args.substr(0, pos);
    string filename = args.substr(pos +1);

    storeRootTreeFile(hashValue, filename);

    cout << "OK\n";
  } else {
    cout << "ERROR syntax error: " << cmdline << "\n";
  }
}
