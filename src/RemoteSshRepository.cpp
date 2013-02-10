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
  if (sshProcess) delete sshProcess;
}


void RemoteSshRepository::open()
{
  cout << "RemoteSshRepository.open()... " << config.remotePart << "\n";
//  char* args[] = {(char*) config.remotePart.c_str(), "touch", "/tmp/ssh-test", 0};
  char* args[] = {"florian@naboo", "touch", "/tmp/ssh-test", 0};
//  char* args[] = {"-la", "/aaaa", 0};
  Process p("ssh", args);//, config.remotePart, "ls");
  p.waitFor();
  printf("return code: %d\n\n", p.exitValue());
  exit(2);

}

void RemoteSshRepository::lock(bool exclusive)
{
}

void RemoteSshRepository::unlock()
{
}


string RemoteSshRepository::storeTreeFile(BackupRun* run, string& treeFile)
{
}

string RemoteSshRepository::storeFile(BackupRun* run, File& srcFile)
{
}

void RemoteSshRepository::storeRootTreeFile(string& rootHashValue)
{
}

vector<TreeFileEntry> RemoteSshRepository::loadTreeFile(string& treeId)
{
}

void RemoteSshRepository::exportFile(TreeFileEntry& entry, OutputStream& out)
{
}

void RemoteSshRepository::exportFile(string& id, OutputStream& out)
{
}

void RemoteSshRepository::exportSymlink(TreeFileEntry& entry, File& linkFile)
{
}

void RemoteSshRepository::show()
{
}


ShabackInputStream RemoteSshRepository::createInputStream()
{
}

