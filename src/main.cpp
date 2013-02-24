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

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "shaback.h"
#include "RuntimeConfig.h"
#include "lib/config.h"
#include "lib/Sha1.h"
#include "lib/Exception.h"

using namespace std;

void showUsage(string& op)
{
#if defined(SHABACK_HAS_BACKUP)
  if (op == "backup") {
    printf("usage: shaback backup [<general_options>] [-n <name> | --name <name>]\n"
      "                      [-t | --totals] [-p <pw> | --password=<pw>]\n"
      "                      [-W | --no-write-cache]\n"
      "                      [<file> ...]\n\n"
      "\tPerforms backup run. If no filenames are specified on the command line,\n"
      "\tfiles and directories are backed up as specified in the config file\n"
      "\tvia the addDir('<file>') command.\n\n"
      "Options:\n"
      "\t-n <name>, --name=<name>\n"
      "\t    Specifies the backup set's name. The name will be reflected as the index\n"
      "\t    file's name prefix.\n\n"
      "\t-t, --totals\n"
      "\t    Give summary report at the end of the backup run.\n\n"
      "\t-p <pw>, --password=<pw>\n"
      "\t    If encryption is enabled, this specifies the password to be used.\n\n"
      "\t-W, --no-write-cache\n"
      "\t    For systems with insufficient RAM: Don't populate write cache.\n\n"
      "\t<file>...\n"
      "\t    An arbitrary number of files and directories to be backed up.\n"
      "\t    If no files are specified here, the directory list from the config\n"
      "\t    file takes effect.\n\n");
  } else
#endif
  if (op == "restore") {
    printf("usage: shaback restore [<general_options>] [-p <pw> | --password=<pw>]\n"
      "                      [-t | --totals] [-S | --skip-existing]\n"
      "                      [-o | --cpio] [-q | --quiet]\n"
      "                      <rootfile> | <dir-id>\n\n");
    printf("\tRestores directories and files from the repository.\n\n"
      "\t<rootfile> is a filename from the repository's index/ directory.\n"
      "\t<dir-id> is the ID of the directory file to be restored.\n\n"
      "\t-S, --skip-existing\n"
      "\t    Skip files already existing in destination directory.\n\n"
      "\t-t, --totals\n"
      "\t    Give summary report at the end of the recovery run.\n\n"
      "\t-o, --cpio\n"
      "\t    Restore to cpio stream on stdout.\n\n"
      "\t-q, --quiet\n"
      "\t    Suppress progress output.\n\n"
      "\tFiles will always be restored into the CWD.\n");
  } else if (op == "gc") {
    printf("usage: shaback gc [<general_options>] [-p <pw> | --password=<pw>]\n\n");
    printf("\tPerforms a garbage collection to delete unused files from the repository.\n\n");
  } else if (op == "show") {
    printf("usage: shaback show [<general_options>] [-p <pw> | --password=<pw>] <id>\n\n");
    printf("\tDecompresses and decrypts the specified object from the repository to stdout.\n\n");
#if defined(SHABACK_HAS_BACKUP)
  } else if (op == "init") {
    printf("usage: shaback init [<general_options>] [-f | --force]\n"
      "                      [-E <enc> | --encryption=<enc>]\n"
      "                      [-C <comp> | --compression=<comp>]\n"
      "                      [-F <fmt> | --repo-format=<fmt>]\n"
      "                      [-p <pw> | --password=<pw>]\n\n"
      "\tCreates a new repository at the location specified in one of the config files\n"
      "\tor via the --repository option. Defaults to the current working directory.\n\n"
      "Options:\n"
      "\t-f, --force\n"
      "\t    Force creation even if the destination directory is not empty.\n\n"
      "\t-E=<enc>, --encryption=<enc>\n"
      "\t    Enable encryption for this repository. You cannot alter this setting\n"
      "\t    after the repository has been created!\n"
      "\t    Be sure to specify a password via the --password option (see below).\n"
      "\t    <enc> must be one of: `None' or `Blowfish'. Defaults to `None'.\n\n"
      "\t-C=<comp>, --compression=<comp>\n"
      "\t    Enable data compression for this repository. You cannot alter this setting\n"
      "\t    after the repository has been created!\n"
      "\t    <comp> must be one of: `None', `BZip', `BZip-1', `BZip-9'"
#if defined(LZMA_FOUND)
           ", `LZMA',\n"
      "\t    `LZMA-0', `LZMA-5', `LZMA-9'"
#endif
           " or `Deflate'.\n"
      "\t    Defaults to `Deflate'.\n\n"
      "\t-F=<fmt>, --repo-format=<fmt>\n"
      "\t    Select an alternative repository format.\n"
      "\t    <fmt> must be one of: `2-2' or `3', where `2-2' is the default.\n\n"
      "\t-p <pw>, --password=<pw>\n"
      "\t    If encryption is enabled, this specifies the password to be used.\n\n");
#endif
  } else if (op == "deflate") {
    printf("usage: shaback deflate\n\n");
  } else if (op == "inflate") {
    printf("usage: shaback inflate\n\n");
  } else {
    printf("usage: shaback <command> [<options>] [<args>]\n");
    printf("\n");
    printf("Valid commands are:\n");
#if defined(SHABACK_HAS_BACKUP)
    printf("   backup      Backup a set of files or directories.\n");
#endif
    printf("   gc          Garbage collection: Delete unused files from archive.\n");
#if defined(SHABACK_HAS_BACKUP)
    printf("   init        Create a new repository.\n");
#endif
    printf("   restore     Restore files from repository.\n");
    //    printf("   cleanup     Delete old index files\n");
    printf("   show        Decompress and decrypt a certain object from the repository.\n");
    printf("   deflate     Compress data from stdin to stdout using `Deflate' compression.\n");
    printf("   inflate     Decompress data from stdin to stdout using `Deflate' compression.\n");
    printf("\n");
    printf("General options are:\n");
    printf("\t-c <file>, --config=<file>\n"
        "\t    Additionally load specified config file.\n\n");
    printf("\t-i <error>, --ignore-error=<error>\n"
        "\t    Ignore errors of specified type. One of:\n"
        "\t    `chown', `chmod', `utime'.\n\n");
    printf("\t-r <dir>, --repository=<dir>\n"
      "\t    Override repository specified in config files.\n\n");
    printf("\t-v, --verbose\n"
      "\t    Be verbose.\n\n");
    printf("\t-d, --debug\n"
      "\t    Be even more verbose.\n\n");
    printf("\t-h, --help\n"
      "\t     Show usage information.\n\n");
    printf("See `shaback <command> --help' for more information on a specific command.\n");
    //  printf("Version: " + VERSION);
  }
}

int main(int argc, char** argv)
{
  try {
    RuntimeConfig config;
    config.load();
    config.parseCommandlineArgs(argc, argv);
    config.finalize();

    if (config.operation.empty()) {
      showUsage(config.operation);
      return 1;
    }

    if (config.help) {
      showUsage(config.operation);
    } else {
      Shaback shaback(config);

      if (config.operation == "init") {
#if defined(SHABACK_HAS_BACKUP)
        shaback.createRepository();
#else
        cerr << "Command 'init' not available - missing openssl." << endl;
        exit(1);
#endif
      } else if (config.operation == "backup") {
#if defined(SHABACK_HAS_BACKUP)
        return shaback.repository.backup();
#else
        cerr << "Command 'backup' not available - missing openssl." << endl;
        exit(1);
#endif
      } else if (config.operation == "restore") {
        return shaback.repository.restore();
      } else if (config.operation == "show") {
        shaback.repository.show();
      } else if (config.operation == "gc") {
        shaback.repository.gc();
      } else if (config.operation == "deflate") {
        return shaback.deflate();
      } else if (config.operation == "inflate") {
        return shaback.inflate();
      } else {
        cerr << "Invalid operation `" << config.operation << "'." << endl;
        return 1;
      }
      return 0;
    }
  } catch (Exception& ex) {
    cerr << ex.getMessage() << endl;
//    config.runErrorCallbacks(ex.getMessage());
    return 10;
  }
}

