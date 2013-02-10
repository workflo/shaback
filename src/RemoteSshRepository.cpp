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
    Repository(config), sshProcess(0)
{
  readBuffer = (char*) malloc(READ_BUFFER_SIZE);
}

RemoteSshRepository::~RemoteSshRepository()
{
  free(readBuffer);
  if (sshProcess) {
    sendCommand("close");
    delete sshProcess;
  }
}


void RemoteSshRepository::open()
{
  char *args[] = {(char*) "ssh", (char*) config.remotePart.c_str(),
      (char*) "/Users/florian/git/shaback/src/shaback", (char*) "-r", (char*) config.repoDir.path.c_str(),
      (char*) "remote", 0};
  sshProcess = new Process("ssh", args);
}

void RemoteSshRepository::lock(bool exclusive)
{
  sendCommand("lock");
}

void RemoteSshRepository::unlock()
{
  sendCommand("unlock");
}


string RemoteSshRepository::storeTreeFile(BackupRun* run, string& treeFile)
{
  sendCommand("unlock");
}

string RemoteSshRepository::storeFile(BackupRun* run, File& srcFile)
{
  sendCommand("unlock");
}

void RemoteSshRepository::storeRootTreeFile(string& rootHashValue)
{
  sendCommand("unlock");
}

vector<TreeFileEntry> RemoteSshRepository::loadTreeFile(string& treeId)
{
  sendCommand("unlock");
}

void RemoteSshRepository::exportFile(TreeFileEntry& entry, OutputStream& out)
{
  sendCommand("unlock");
}

void RemoteSshRepository::exportFile(string& id, OutputStream& out)
{
  sendCommand("unlock");
}

void RemoteSshRepository::exportSymlink(TreeFileEntry& entry, File& linkFile)
{
  sendCommand("unlock");
}

void RemoteSshRepository::show()
{
  sendCommand("unlock");
}


ShabackInputStream RemoteSshRepository::createInputStream()
{
}

void RemoteSshRepository::sendCommand(string command)
{
  cout << ">> " << command << endl;

  OutputStream *out = sshProcess->getOutputStream();

  out->write(command);
  out->write('\n');

//  p.waitFor();
//  printf("return code: %d\n\n", p.exitValue());
//  exit(2);
}


int RemoteSshRepository::remoteCommandListener()
{
  cerr << "Cannot start remote command listener for remote repository." << endl;
  exit(4);
}
