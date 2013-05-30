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
#include <math.h>

#ifdef WIN32
# include "getopt.h"
#else
# include <getopt.h>
# include <fnmatch.h>
#endif

#include "lib/config.h"
#include "lib/FileInputStream.h"
#include "lib/Exception.h"

using namespace std;

void showUsage(const char* arg0)
{
  printf("Usage: %s [-b chunksize] [-h] [-n] <oldfile> <newfile> \n\n", arg0);
  printf("\t    -b <chunksize> in bytes\n");
  printf("\t    -h html output\n");
  printf("\t    -n do not list unchanged blocks\n");
}


static bool html = false;
static bool listUnchangedBlocks = true;
static int chunkSize = 1024 * 1024;
static string oldFile;
static string newFile;


void parseCommandlineArgs(int argc, char** argv)
{
  while (true) {
    int option_index = 0;
    static struct option long_options[] = { { "chunksize", required_argument, 0, 'b' }, { "html", no_argument, 0, 'h' }, {
        "no-unchanged", no_argument, 0, 'n' },
        { 0, 0, 0, 0 } };

    int c = getopt_long(argc, argv, "b:hn", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
      case 'h':
        html = true;
        break;

      case 'n':
        listUnchangedBlocks = false;
        break;

      case 'b':
        chunkSize = atoi(optarg);
        break;

      default:
        cerr << "?? getopt returned character code " << c << "??" << std::endl;
        break;
    }
  }

  if (argc != optind + 2) {
    cerr << "Wrong number of arguments." << endl;
    showUsage(argv[0]);
    exit(1);
  } else {
    oldFile = argv[optind++];
    newFile = argv[optind++];
  }
}


#define CHANGETYPE_CHANGE 1
#define CHANGETYPE_GROWN 2
#define CHANGETYPE_SHRUNK 3

void run()
{
  FileInputStream oldFileIn(oldFile);
  FileInputStream newFileIn(newFile);
  long numSame = 0;
  long numChange = 0;
  long numGrown = 0;
  long numShrunk = 0;
  long oldtotal = 0;
  long newtotal = 0;
  long smetotal = 0;
  long chgtotal = 0;
  long blktotal = 0;
  int numchunks = 0;
  bool oldend = false;
  bool newend = false;
  char* oldbuf = (char*) malloc(chunkSize);
  char* newbuf = (char*) malloc(chunkSize);

  while (!oldend || !newend) {
    int type = CHANGETYPE_CHANGE;
    int oldread, newread;

    if (oldend) {
      type = CHANGETYPE_GROWN;
      numGrown++;
    } else {
      oldread = oldFileIn.read(oldbuf, chunkSize);
      //print "old: $oldbuf\n";
      oldend = (oldread != chunkSize);
      oldtotal += oldread;
    }

    if (newend) {
      type = CHANGETYPE_SHRUNK;
      numShrunk++;
    } else {
      newread = newFileIn.read(newbuf, chunkSize);
      //print "new: $newbuf\n";
      newend = (newread != chunkSize);
      newtotal += newread;
    }

    if (type == CHANGETYPE_CHANGE) {
      int cmpsize = oldread < newread ? oldread : newread;
      blktotal += cmpsize;
      int cmpdiff = 0;
      for (int x = 0; x < cmpsize; x++) {
        if (oldbuf[x] != newbuf[x]) cmpdiff ++;
      }

      if (cmpdiff == 0) {
        smetotal += cmpsize;
        numSame++;
        // Gleicher Block
        if (html) {
          printf ("<div class=\"c e\"></div>");
        } else {
          if (listUnchangedBlocks) {
            printf ("%d\tEQ\n", numchunks);
          }
        }
      } else {
        chgtotal += cmpdiff;
        numChange++;
        // Anderer Block
        int cl = ceil(cmpdiff * 100/ cmpsize);
        if (html) {
          printf ("<div class=\"c c%i\"></div>", cl);
        } else {
          printf ("%d\t%d\t%i\n", numchunks, cmpdiff, cl);
        }
      }
    } else if (type == CHANGETYPE_GROWN) {
      // Grown
      if (html) {
        printf ("<div class=\"c g\"></div>");
      } else {
        printf ("%d\tGROWN\n", numchunks);
      }
    } else if (type == CHANGETYPE_SHRUNK) {
      // Shrunk
      if (html) {
        printf ("<div class=\"c s\"></div>");
      } else {
        printf ("%d\tSHRUNK\n", numchunks);
      }
    } else {
      exit(10);
    }

    numchunks++;
  }
};


int main(int argc, char** argv)
{
  try {
    parseCommandlineArgs(argc, argv);
    run();
  } catch (Exception& ex) {
    cerr << ex.getMessage() << endl;
    return 2;
  }
}

