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

#ifdef WIN32
# include "getopt.h"
#else
# include <getopt.h>
# include <fnmatch.h>
#endif

#include "lib/config.h"
#include "lib/Sha1.h"
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
//    while (optind < argc) {
//      if (operation.empty()) {
//        // First argument is OPERATION:
//        operation = argv[optind++];
//      } else {
//        cliArgs.push_back(argv[optind++]);
//      }
//    }
  }

}


int main(int argc, char** argv)
{
  try {
    parseCommandlineArgs(argc, argv);
  } catch (Exception& ex) {
    cerr << ex.getMessage() << endl;
//    config.runErrorCallbacks(ex.getMessage());
    return 10;
  }
}

