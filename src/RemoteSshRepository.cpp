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
#include "lib/Process.h"

#include "BackupRun.h"
#include "GarbageCollection.h"
#include "RemoteSshRepository.h"
#include "RestoreRun.h"
#include "ShabackInputStream.h"
#include "ShabackOutputStream.h"
#include "ShabackException.h"
#include "SplitFileIndexReader.h"
#include "TreeFile.h"

#define READ_BUFFER_SIZE (1024 * 4)

using namespace std;

RemoteSshRepository::RemoteSshRepository(RuntimeConfig& config) :
    Repository(config), sshProcess(0), remoteIn(0), remoteOut(0)
{
  readBuffer = (char*) malloc(READ_BUFFER_SIZE);
}

RemoteSshRepository::~RemoteSshRepository()
{
  free(readBuffer);
  if (sshProcess) {
    string response;
    sendCommand("close", response);
    delete sshProcess;
  }
}


void RemoteSshRepository::open()
{
  char *args[] = {(char*) "ssh", (char*) config.remotePart.c_str(),
      (char*) "/Users/florian/git/shaback/src/shaback", /*(char*) "-r", (char*) config.repoDir.path.c_str(),*/
      (char*) "remote", 0};
  sshProcess = new Process("ssh", args);
  remoteOut = sshProcess->getOutputStream();
  remoteIn = sshProcess->getInputStream();

  string hello;
  if (!remoteIn->readLine(hello)) {
    throw IOException("Lost connection to remote shaback.");
  }

  if (hello.find("SHABACK ") != 0) {
    throw IOException(string("Unexpected response from remote shaback: ").append(hello));
  }
}

void RemoteSshRepository::lock(bool exclusive)
{
  string response;
  sendCommand("lock", response);
}

void RemoteSshRepository::unlock()
{
  string response;
  sendCommand("unlock", response);
}

void RemoteSshRepository::storeTextFile(string hashValue, string content)
{
  string response;
  sendCommand((string("store ").append(hashValue)), response);

  // Send string length as block size:
  char cmd[32];
#ifdef __APPLE__
  sprintf(cmd, "%jd", content.size());
#else
  sprintf(cmd, "%lu", content.size());
#endif
  sendCommand(cmd, response);

  // Send whole string in one block:
  printf("sending %i chars...\n", content.size());
  remoteOut->write(content);
  waitForOk("store", response);

  // No more blocks to come:
  sendCommand("0", response);
}

void RemoteSshRepository::storeRootTreeFile(string rootHashValue, string filename)
{
  string response;
  sendCommand(string("storeRootTreeFile ").append(rootHashValue).append(" ").append(filename), response);
}

bool RemoteSshRepository::contains(string& hashValue)
{
  string response;
  sendCommand(string("contains ").append(hashValue), response);

  return response == "1";
}

void RemoteSshRepository::store(BackupRun* run, File& srcFile, InputStream& in, string& hashValue)
{
  string response;
  sendCommand((string("store ").append(hashValue)), response);

  while (true) {
    int bytesRead = in.read(readBuffer, READ_BUFFER_SIZE);
    if (bytesRead == -1)
      break;

    // Send string length as block size:
    char cmd[32];
    sprintf(cmd, "%d", bytesRead);
    sendCommand(cmd, response);

    printf("sending %i bytes...\n", bytesRead);
    remoteOut->write(readBuffer, bytesRead);
    waitForOk("store", response);
  }

  // No more blocks to come:
  sendCommand("0", response);
}

vector<TreeFileEntry> RemoteSshRepository::loadTreeFile(string& treeId)
{
  string response;
  sendCommand("loadTreeFile", response);
}

void RemoteSshRepository::exportFile(TreeFileEntry& entry, OutputStream& out)
{
  string response;
  sendCommand("exportFile", response);
}

void RemoteSshRepository::exportFile(string& id, OutputStream& out)
{
  string response;
  sendCommand("exportFile", response);
}

void RemoteSshRepository::exportSymlink(TreeFileEntry& entry, File& linkFile)
{
  string response;
  sendCommand("exportSymlink", response);
}

void RemoteSshRepository::show()
{
  string response;
  sendCommand("show", response);
}

void RemoteSshRepository::deleteOldIndexFiles()
{
//  string response;
//  sendCommand("deleteOldIndexFiles", response);
}


ShabackInputStream RemoteSshRepository::createInputStream()
{
}

void RemoteSshRepository::sendCommand(string command, string& response)
{
  cout << ">> " << command << endl;

  remoteOut->write(string(command).append("\n"));

  waitForOk(command, response);
}

void RemoteSshRepository::waitForOk(string command, string& response)
{
  if (!remoteIn->readLine(response)) {
    throw IOException(string("Lost connection to remote shaback upon command: ").append(command));
  }
  if (response.find("OK ") == 0) {
    response = response.substr(3);
  } else if (response.find("OK") == 0) {
    response = "";
  } else {
    if (response.find("ERROR ") == 0)
      throw IOException(response.substr(6));
    else
      throw IOException(response);
  }
  cout << "<< " << response << endl;
}


int RemoteSshRepository::remoteCommandListener()
{
  cerr << "Cannot start remote command listener for remote repository." << endl;
  exit(4);
}
