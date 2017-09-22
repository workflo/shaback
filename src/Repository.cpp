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
#include "lib/Digest.h"
#include "lib/Sha1.h"
#include "lib/Sha256.h"

#include "shaback.h"
#include "BackupRun.h"
#include "GarbageCollection.h"
#include "History.h"
#include "Migration.h"
#include "Repository.h"
#include "RestoreRun.h"
#include "ShabackInputStream.h"
#include "ShabackOutputStream.h"
#include "ShabackException.h"
#include "SplitFileIndexReader.h"
#include "TreeFile.h"


#define READ_BUFFER_SIZE (1024 * 4)

using namespace std;

Repository::Repository(RuntimeConfig& config) :
    config(config), splitBlockSize(1024 * 1024 * 5), splitMinBlocks(5)
{
  readBuffer = (char*) malloc(max(READ_BUFFER_SIZE, splitBlockSize));
}

Repository::~Repository()
{
  free(readBuffer);
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
  repoFormat = repoFormatByName(props.getProperty("repoFormat"));

  // Check Repo Version
  version = props.getProperty("version");
  if (config.operation != "migrate") {
    if (version == "2") {
      throw Exception("Unsupported repository version \"2\". Migrate your repository to version \"3\" by running" \
        "\n\tshaback migrate");
    } else if (version != SHABACK_REPO_VERSION) {
      throw Exception(string("Unsupported repository version \"").append(version).append("\"."));
    }
  }
  
  if (encryptionAlgorithm != ENCRYPTION_NONE) {
    if (config.cryptoPassword.empty())
      throw MissingCryptoPassword();
#if defined(OPENSSL_FOUND)
    checkPassword();
#else
    cerr << "Cannot handle encrypted repositories - missing openssl." << endl;
    exit(1);
#endif
  }
}

#if defined(OPENSSL_FOUND)
void Repository::checkPassword()
{
  FileInputStream in(config.passwordCheckFile);
  string hashFromFile;
  string hashFromPassword = hashPassword(encryptionAlgorithm, config.cryptoPassword);

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
#endif

void Repository::lock(bool exclusive)
{
  if (config.lockCount > 0) {
    if (exclusive && !config.haveExclusiveLock) throw LockingException("Upgrading a non-exclusive lock to an exclusive lock is not implemented.");
    // Be reentrant:
    config.lockCount++;
    return;
  }

  string existingLocks = config.locksDir.listFilesToString("*.lock", "; ");

  int lockFileFh = ::open(config.lockFile.path.c_str(), O_CREAT | O_EXCL, S_IRWXU | S_IROTH | S_IRGRP);
  if (lockFileFh == -1)
    throw Exception::errnoToException(config.lockFile.path);

  if (config.useSymlinkLock) {
    int ret = ::symlink(config.lockFile.fname.c_str(), config.exclusiveLockFile.path.c_str());
    if (ret != 0) {
      if (errno == EEXIST) {
        throw LockingException(
            string("Repository is locked exclusively. Check lock files in ").append(config.locksDir.path).append(": ").append(existingLocks));
      } else if (errno == ENOSYS) {
        throw LockingException("Destination filesystem does not support symlinks. Use `-L' to lock without symlinks.");
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
                config.locksDir.path).append(": ").append(existingLocks));
      }
    } else {
      config.exclusiveLockFile.remove();
    }
  } else {
    if (exclusive) {
      throw LockingException(string("Cannot aquire exclusive lock with --no-symlink-lock."));
    }
    if (File(config.exclusiveLockFile).isFile()) {
      throw LockingException(string("Repository is locked exclusively. Check lock files in ")
        .append(config.locksDir.path).append(": ").append(existingLocks));
    }
  }

  config.lockCount = 1;
}

void Repository::unlock(bool force)
{
  if (config.lockCount > 1 && !force) {
    config.lockCount--; 
    return;
  }

  config.lockCount = 0;
  config.lockFile.remove();
  if (config.haveExclusiveLock) {
    config.exclusiveLockFile.remove();
  }
}

int Repository::backup()
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

File Repository::hashValueToFile(string hashValue)
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

bool Repository::contains(string& hashValue)
{
  return writeCache.count(hashValue) || hashValueToFile(hashValue).isFile();
}

string Repository::storeTreeFile(BackupRun* run, string& treeFile)
{
  throw(UnsupportedOperation("Repository::storeTreeFile"));
  Sha1 sha1;
  sha1.update(treeFile);
  sha1.finalize();
  string hashValue = sha1.toString();

  if (!contains(hashValue)) {
    File file = hashValueToFile(hashValue);

    if (config.debug) {
      cout << "[t] " << file.path << endl;
    }

    ShabackOutputStream os = createOutputStream();
    os.open(file);
    os.write(treeFile);
    os.finish();

    run->numBytesStored += treeFile.size();
  }

  writeCache.insert(hashValue);

  return hashValue;
}

string Repository::storeFile(BackupRun* run, File& srcFile, intmax_t* totalFileSize)
{
  run->numFilesRead++;
  *totalFileSize = srcFile.getSize();
  run->numBytesRead += *totalFileSize;

  string hashValue = srcFile.getXAttr("user.shaback.sha1");

  // XAttr contains sha1 digest and mtime is still uptodate:
  if (!hashValue.empty() && strtol(srcFile.getXAttr("user.shaback.mtime").c_str(), 0, 10) == srcFile.getPosixMtime()) {
    // Hash value is already stored:
    if (contains(hashValue)) {
      // Done :)
      return hashValue;
    }
  }

  FileInputStream in(srcFile);

  // Allow large files with certain name patterns to be split into blocks/chunks:
  if (config.splitFile(srcFile)) {
    return storeSplitFile(run, srcFile, in, totalFileSize);
  }

  Sha1 sha1;
  while (true) {
    int bytesRead = in.read(readBuffer, READ_BUFFER_SIZE);
    if (bytesRead == -1)
      break;
    sha1.update((unsigned char*) readBuffer, bytesRead);
  }

  sha1.finalize();
  hashValue = sha1.toString();

  srcFile.setXAttr("user.shaback.sha1", hashValue); // TODO: Use dynamic digest name
  srcFile.setXAttr("user.shaback.mtime", srcFile.getPosixMtime());

  if (!contains(hashValue)) {
    in.reset();
    *totalFileSize = 0;

    File destFile = hashValueToFile(hashValue);

    if (config.verbose || config.debug) {
      cout << config.color_filename << "[m] " << srcFile.path << config.color_default << endl;
      if (config.debug) {
        cout << "[f] " << destFile.path << endl;
      }
    }

    ShabackOutputStream os = createOutputStream();
    os.open(destFile);

    while (true) {
      int bytesRead = in.read(readBuffer, READ_BUFFER_SIZE);
      if (bytesRead == -1)
        break;
      os.write(readBuffer, bytesRead);
      run->numBytesStored += bytesRead;
      *totalFileSize += bytesRead;
    }

    os.finish();

    run->numFilesStored++;
  } else {
    if (config.debug) {
      cout << "[ ] " << srcFile.path << endl;
    }
  }

  writeCache.insert(hashValue);

  return hashValue;
}

string Repository::storeSplitFile(BackupRun* run, File &srcFile, InputStream &in,
    intmax_t* totalFileSize)
{
  string blockList;
  Sha1 totalSha1;
  int blockCount = 0;

  if (config.verbose || config.debug) {
    cout << config.color_filename << "[s] " << srcFile.path << config.color_default << endl;
  }

  *totalFileSize = 0;

  while (true) {
    Sha1 blockSha1;
    const int bytesRead = in.read(readBuffer, splitBlockSize);
    if (bytesRead == -1)
      break;

    blockSha1.update((unsigned char*) readBuffer, bytesRead);
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

    blockList.append(blockHashValue);
    blockList.append("\n");
    *totalFileSize += bytesRead;

    totalSha1.update((unsigned char*) readBuffer, bytesRead);
  }

  if (config.verbose)
    cout << endl;

  totalSha1.finalize();
  string totalHashValue = totalSha1.toString();
  totalHashValue.append(SPLITFILE_ID_INDICATOR_STR);

  if (!contains(totalHashValue)) {
    File blockFile = hashValueToFile(totalHashValue);
    ShabackOutputStream os = createOutputStream();
    os.open(blockFile);
    os.write(blockList);
    os.finish();

    writeCache.insert(totalHashValue);
    run->numBytesStored += blockList.size();
    run->numFilesStored++;
  }

  srcFile.setXAttr("user.shaback.sha1", totalHashValue); // TODO: Use dynamic digest name
  srcFile.setXAttr("user.shaback.mtime", srcFile.getPosixMtime());

  return totalHashValue;
}

vector<TreeFileEntry> Repository::loadTreeFile(string& treeId)
{
  string content;

  File file = hashValueToFile(treeId);
  ShabackInputStream in = createInputStream();
  in.open(file);

  in.readAll(content);

  vector<TreeFileEntry> list;
  int from = 0;
  int until;

  if ((until = content.find('\n', from)) == string::npos) {
    if (config.verbose) {
      cerr << config.color_error;
      cerr << "Missing header line in index file " << hashValueToFile(treeId).path << ":" << endl;
      cerr << config.color_low;
      cerr << content.substr(0, 200) << (content.size() > 200 ? "..." : "") << endl;
      cerr << config.color_default;
    }
    throw InvalidTreeFile(string("Missing header line in index ") + treeId);
  }
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

#if defined(HAVE_UNORDERED_SET)
  unordered_set<string>::iterator it;
#else
  set<string>::iterator it;
#endif

  for (it = writeCache.begin(); it != writeCache.end(); it++) {
    string s(*it);
    writer.write(s);
    writer.newLine();
  }
}

void Repository::importCacheFile()
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


RestoreReport Repository::restore()
{
  if (config.cliArgs.empty()) {
    throw RestoreException("Don't know what to restore.");
  }

  string shabackupFilename = config.cliArgs.front();
  config.cliArgs.pop_front();

  open();

  File shabackupFile(selectShabackupFile(shabackupFilename));

  return restore(shabackupFile, config.cliArgs, false);
}


File Repository::selectShabackupFile(string filename)
{
  if (filename.rfind(".shabackup") == filename.size() - 10) {
    string fname = filename.substr(filename.rfind(File::separator) + 1);
    return File(config.indexDir, fname);
  } else {
    throw RestoreException(string("Don't know how to restore `").append(filename).append("'."));
  }
}


RestoreReport Repository::testRestore()
{
  if (!config.all && config.cliArgs.empty()) {
    throw RestoreException("Don't know what to restore.");
  }

  open();

  if (config.all) {
    RestoreReport report;
   
    vector<File> directoryFiles = config.indexDir.listFiles("*.shabackup");

    for (vector<File>::iterator it = directoryFiles.begin(); it < directoryFiles.end(); it++) {
      File file(*it);
      if (config.verbose) cerr << config.color_low << "* " << file.path << config.color_default << endl;

      File shabackupFile(selectShabackupFile(file.path));
  
      RestoreReport r = restore(shabackupFile, list<string>(), true);
      
      report.numErrors += r.numErrors;

      if (r.hasErrors()) {
        cerr << config.color_error << "ERROR DETECTED: " << file.path << " contains errors!" << config.color_default << endl;
      }
    }

    if (config.showTotals) {
      fprintf(stderr, "\nTotal Errors:         %12d\n", report.numErrors);
    }

    return report;
  } else {
    string shabackupFilename = config.cliArgs.front();
    config.cliArgs.pop_front();

    File shabackupFile(selectShabackupFile(shabackupFilename));
    
    return restore(shabackupFile, list<string>(), true);
  }
}

RestoreReport Repository::restore(File shabackupFile, list<string> files, bool testRestore)
{
  RestoreRun run(config, *this, shabackupFile, testRestore);
  File destinationDir(".");

  if (files.empty()) files.push_back(""); // Restore everything!

  return run.start(files, destinationDir);
}

void Repository::exportFile(TreeFileEntry& entry, OutputStream& out)
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

void Repository::exportFile(string& id, OutputStream& out)
{
  File inFile = hashValueToFile(id);
  ShabackInputStream in = createInputStream();
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

void Repository::testExportFile(RestoreRun& restoreRun, TreeFileEntry& entry)
{
  Sha1 sha1;
  intmax_t totalBytesRead = 0;

  if (entry.isSplitFile) {
    try {
      SplitFileIndexReader reader(*this, entry.id);
      string hashValue;

      while (reader.next(hashValue)) {
        File blockFile = hashValueToFile(hashValue);

        if (config.quick && blockFile.isFile()) continue;

        try {
          ShabackInputStream in = createInputStream();
          in.open(blockFile);

          // Read file, count bytes and calculate actual hash digest:
          while (true) {
            int bytesRead = in.read(readBuffer, READ_BUFFER_SIZE);
            if (bytesRead == -1)
              break;
            sha1.update((const unsigned char*) readBuffer, bytesRead);
            totalBytesRead += bytesRead;
          }
        } catch (Exception &ex) {
          cerr << config.color_error << "FAILED: " << entry.path << ": Error reading " << hashValue << ": " << ex.getMessage() << config.color_default << endl;
          restoreRun.report.numErrors ++;
        }
      }
    } catch (Exception &ex) {
      cerr << config.color_error << "FAILED: " << entry.path << ": Error reading block list " << entry.id << ": " << ex.getMessage() << config.color_default << endl;
      restoreRun.report.numErrors ++;
      return;
    }
  } else {
    File inFile = hashValueToFile(entry.id);

    if (!config.quick || !inFile.isFile()) {
      try {
        ShabackInputStream in = createInputStream();
        in.open(inFile);

        // Read file, count bytes and calculate actual hash digest:
        while (true) {
          int bytesRead = in.read(readBuffer, READ_BUFFER_SIZE);
          if (bytesRead == -1)
            break;
          sha1.update((const unsigned char*) readBuffer, bytesRead);
          totalBytesRead += bytesRead;
        }
      } catch (Exception &ex) {
        cerr << config.color_error << "FAILED: " << entry.path << ": Error reading " << entry.id << ": " << ex.getMessage() << config.color_default << endl;
        restoreRun.report.numErrors ++;
        return;
      }
    }
  }

  if (config.quick) {
    // Nothing more to check for 'quick' mode.
  } else {
    // Check actual file size and hash digest:
    if (totalBytesRead != entry.size) {
      cerr << config.color_error << "FAILED: " << entry.path << ": size mismatch (" << totalBytesRead << " <> " << entry.size << ")" << config.color_default << endl;
      restoreRun.report.numErrors ++;
      return;
    } else {
      sha1.finalize();
      string hashValue = sha1.toString();

      if (hashValue != entry.id.substr(0, 40)) {
        cerr << config.color_error << "FAILED: " << entry.path << ": hash mismatch (" << hashValue << " <> " << entry.id << ")" << config.color_default << endl;
        restoreRun.report.numErrors ++;
        return;
      }
    }
  }

  if (config.verbose >= 2) cerr << config.color_debug << "OK: " << entry.path << config.color_default << endl;
  restoreRun.report.numFilesRestored ++;
  restoreRun.report.numBytesRestored += totalBytesRead;
}


void Repository::show()
{
  if (config.cliArgs.empty()) {
    throw RestoreException("Don't know what to show.");
  }

  open();

  string id = config.cliArgs.front();
  StandardOutputStream out(stdout);
  exportFile(id, out);
}

void Repository::gc()
{
  open();
  GarbageCollection gc(config, *this);
  gc.run();
}

void Repository::history()
{
  History history(config, *this);
  history.run();
}

int Repository::encryptionByName(string name)
{
  string lcName(name);
  std::transform(lcName.begin(), lcName.end(), lcName.begin(), ::tolower);

  if (lcName == "blowfish") {
    return ENCRYPTION_BLOWFISH;
  } else if (lcName == "aes" || lcName == "aes256") {
    return ENCRYPTION_AES256;
  } else if (lcName == "none" || name.empty()) {
    return ENCRYPTION_NONE;
  } else {
    throw UnsupportedEncryptionAlgorithm(lcName);
  }
}

int Repository::compressionByName(string name)
{
  string lcName(name);
  std::transform(lcName.begin(), lcName.end(), lcName.begin(), ::tolower);

  if (lcName == "deflate") {
    return COMPRESSION_DEFLATE;
  } else if (lcName == "bz" || lcName == "bzip" || lcName == "bzip-5") {
    return COMPRESSION_BZip5;
  } else if (lcName == "bz1" || lcName == "bzip-1" || lcName == "bzip1") {
    return COMPRESSION_BZip1;
  } else if (lcName == "bz9" || lcName == "bz9" || lcName == "bzip-9" || lcName == "bzip9") {
    return COMPRESSION_BZip9;
#if defined(ZSTD_FOUND)
  } else if (lcName == "zstd1" || lcName == "zstd-1" || lcName == "zstd") {
    return COMPRESSION_ZSTD1;
  } else if (lcName == "zstd5" || lcName == "zstd-5") {
    return COMPRESSION_ZSTD5;
  } else if (lcName == "zstd9" || lcName == "zstd-9") {
    return COMPRESSION_ZSTD9;
#endif
#if defined(LZMA_FOUND)
  } else if (lcName == "lzma0" || lcName == "lzma-0") {
    return COMPRESSION_LZMA0;
  } else if (lcName == "lzma5" || lcName == "lzma-5" || lcName == "lzma") {
    return COMPRESSION_LZMA5;
  } else if (lcName == "lzma9" || lcName == "lzma-9") {
    return COMPRESSION_LZMA9;
#endif
  } else if (lcName == "none" || lcName.empty()) {
    return COMPRESSION_NONE;
  } else {
    throw UnsupportedCompressionAlgorithm(name);
  }
}

int Repository::repoFormatByName(string name)
{
  if (name == "default" || name == "standard" || name == "2-2" || name == "") {
    return REPOFORMAT_2_2;
  } else if (name == "3") {
    return REPOFORMAT_3;
  } else {
    throw UnsupportedRepositoryFormat(name);
  }
}

string Repository::compressionToName(int compression)
{
  switch (compression) {
    case COMPRESSION_DEFLATE:
      return "Deflate";
    case COMPRESSION_BZip5:
      return "BZip";
    case COMPRESSION_BZip1:
      return "BZip-1";
    case COMPRESSION_BZip9:
      return "BZip-9";
#if defined(ZSTD_FOUND)
    case COMPRESSION_ZSTD5:
      return "ZStd";
    case COMPRESSION_ZSTD1:
      return "ZStd-1";
    case COMPRESSION_ZSTD9:
      return "ZStd-9";
#endif
#if defined(LZMA_FOUND)
    case COMPRESSION_LZMA0:
      return "Lzma-0";
    case COMPRESSION_LZMA5:
      return "Lzma-5";
    case COMPRESSION_LZMA9:
      return "Lzma-9";
#endif
    default:
      return "None";
  }
}

string Repository::encryptionToName(int encryption)
{
  switch (encryption) {
    case ENCRYPTION_BLOWFISH:
      return "Blowfish";
    case ENCRYPTION_AES256:
      return "AES";
    default:
      return "None";
  }
}

string Repository::repoFormatToName(int fmt)
{
  switch (fmt) {
    case REPOFORMAT_3:
      return "3";
    default:
      return "2-2";
  }
}

#if defined(OPENSSL_FOUND)
  #include <openssl/evp.h>
  #include "lib/KeyDerivation.h"

string Repository::hashPassword(int encryptionAlgorithm, string password)
{
  Sha256 sha;
  string salt(PASSWORDFILE_SALT);
  sha.update(salt);

  switch (encryptionAlgorithm) {
    case ENCRYPTION_BLOWFISH:
      sha.update(password);
      break;
    case ENCRYPTION_NONE:
      break;
    default:
      unsigned char* key = KeyDerivation::deriveFromPassword(password);
      sha.update(&key[EVP_MAX_KEY_LENGTH - 16], 16);
      free(key);
  }
  sha.finalize();
  return sha.toString();
}
#endif

void Repository::removeAllCacheFiles()
{
  vector<File> cacheFiles = config.cacheDir.listFiles("*.scache");

  for (vector<File>::iterator it = cacheFiles.begin(); it < cacheFiles.end(); it++) {
    File cacheFile(*it);
    cacheFile.remove();
  }
}

ShabackInputStream Repository::createInputStream()
{
  return ShabackInputStream(config, compressionAlgorithm, encryptionAlgorithm);
}

ShabackOutputStream Repository::createOutputStream()
{
  return ShabackOutputStream(config, compressionAlgorithm, encryptionAlgorithm);
}


void Repository::migrate()
{
  Migration migration(config, *this);
  migration.run();
}
