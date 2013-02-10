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


string RemoteSshRepository::storeTreeFile(BackupRun* run, string& treeFile)
{
  string response;
  sendCommand("storeTreeFile", response);
}

string RemoteSshRepository::storeFile(BackupRun* run, File& srcFile)
{
  string response;
  sendCommand("storeTreeFile", response);
}

void RemoteSshRepository::storeRootTreeFile(string& rootHashValue)
{
  string response;
  sendCommand("storeTreeFile", response);
}

vector<TreeFileEntry> RemoteSshRepository::loadTreeFile(string& treeId)
{
  string response;
  sendCommand("storeTreeFile", response);
}

void RemoteSshRepository::exportFile(TreeFileEntry& entry, OutputStream& out)
{
  string response;
  sendCommand("storeTreeFile", response);
}

void RemoteSshRepository::exportFile(string& id, OutputStream& out)
{
  string response;
  sendCommand("storeTreeFile", response);
}

void RemoteSshRepository::exportSymlink(TreeFileEntry& entry, File& linkFile)
{
  string response;
  sendCommand("storeTreeFile", response);
}

void RemoteSshRepository::show()
{
  string response;
  sendCommand("storeTreeFile", response);
}


ShabackInputStream RemoteSshRepository::createInputStream()
{
}

void RemoteSshRepository::sendCommand(string command, string& response)
{
  cout << ">> " << command << endl;

  remoteOut->write(string(command).append("\n"));

  if (!remoteIn->readLine(response)) {
    throw IOException(string("Lost connection to remote shaback upon command: ").append(command));
  }
  cout << "<< " << response << endl;

//  sshProcess->waitFor();
//  printf("return code: %d\n\n", sshProcess->exitValue());
//  exit(2);
}


int RemoteSshRepository::remoteCommandListener()
{
  cerr << "Cannot start remote command listener for remote repository." << endl;
  exit(4);
}
