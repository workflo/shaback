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

#include <signal.h>
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
  if (op == "backup") {
    printf("usage: shaback backup [<general_options>] [-n <name> | --name <name>]\n"
      "                      [-t | --totals] [-p <pw> | --password=<pw>]\n"
      "                      [-W | --no-write-cache] [-L | --no-symlink-lock]\n"
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
      "\t-L, --no-symlink-lock\n"
      "\t    For repository file systems that do not support symlinks.\n"
      "\t    Garbage collection will not be supported.\n\n"
      "\t<file>...\n"
      "\t    An arbitrary number of files and directories to be backed up.\n"
      "\t    If no files are specified here, the directory list from the config\n"
      "\t    file takes effect.\n\n");
  } else if (op == "restore") {
    printf("usage: shaback restore [<general_options>] [-p <pw> | --password=<pw>]\n"
      "                      [-t | --totals] [-S | --skip-existing]\n"
      "                      [-o | --cpio] [-q | --quiet] [-G | --gauge]\n");
    printf("                      <shabackup-file> [directories | files]\n\n");
    printf("\tRestores directories and files from the repository.\n\n"
      "\t<shabackup-file> is a filename from the repository's index/ directory.\n"
      "\t[directories | files] are the absolute paths of directories or files to be restored.\n\n"
      "\t-S, --skip-existing\n"
      "\t    Skip files already existing in destination directory.\n\n"
      "\t-t, --totals\n"
      "\t    Give summary report at the end of the recovery run.\n\n"
      "\t-o, --cpio\n"
      "\t    Restore to cpio stream on stdout.\n"
      "\t    Limits max file size to (%jd Byte) 8 GB.\n\n"
      "\t-O, --shaback\n"
      "\t    Restore to shaback recovery stream on stdout.\n\n"
      "\t-q, --quiet\n"
      "\t    Suppress progress output.\n\n"
      "\t-G, --gauge\n"
      "\t    Produce output on stdout suitable for dialog --gauge.\n\n", CPIO_ODC_MAX_FILE_SIZE);
    printf("\tFiles will always be restored into the current working directory.\n");
  } else if (op == "test-restore") {
    printf("usage: shaback test-restore [<general_options>] [-p <pw> | --password=<pw>]\n"
      "                      [-t | --totals] [-Q | --quick]\n");
    printf("                      <shabackup-file>  [-a | --all]\n\n");
    printf("\tPretends to restore files from the repository.\n"
      "\tChecks sizes and hash digests to ensure the backup set's integrity.\n\n"
      "\t<shabackup-file> is a filename from the repository's index/ directory.\n\n"
      "\t-a, --all\n"
      "\t    Check all backup sets from the repo's index/ directory.\n\n"
      "\t-t, --totals\n"
      "\t    Give summary report at the end of the recovery run.\n\n"
      "\t-Q, --quick\n"
      "\t    Just check existence of all files, don't recalculate hashes.\n\n");
  } else if (op == "gc") {
    printf("usage: shaback gc [<general_options>] [-p <pw> | --password=<pw>]\n\n");
    printf("\tPerforms a garbage collection to delete unused files from the repository.\n\n");
  } else if (op == "prune") {
    printf("usage: shaback prune [<general_options>] [-n <name> | --name <name>]\n"
      "                      [-a | --all]\n"
      "                      [-R | --dry-run]\n\n"
      "\tDeletes old index files for a certain or all backup sets according\n"
      "\tto the deletion rules defined by setKeepOldBackupsBoundaries().\n\n"
      "Options:\n"
      "\t-n <name>, --name=<name>\n"
      "\t    Specifies the backup set's name. The name will be reflected as the index\n"
      "\t    file's name prefix.\n"
      "\t-a, --all\n"
      "\t    Prune all backup sets, not just the named one.\n"
      "\t-R, --dry-run\n"
      "\t    Do not actually delete anything, just print out what would be deleted.\n\n");
  } else if (op == "history") {
    printf("usage: shaback history [<general_options>] [-n <name> | --name <name>]\n"
      "                      [-l | --list] [-k <num> | --keep=<num>]\n"
      "                      [-D | --details]\n"
      "                      [-1]\n\n"
      "\tPerforms operations to view or maintain the backup history.\n\n"
      "Actions:\n"
      "\t-l, --list\n"
      "\t    Lists available backups/versions for the selected backup set.\n\n"
      "\t-k <num>, --keep=<num>\n"
      "\t    Specifies the number of backups/versions to keep.\n"
      "\t    The latest <num> backups will be preserved,\n"
      "\t    excessive (older) backups will be deleted!\n\n"
      "\t-D, --details\n"
      "\t    Displays detailed human-readable information about the latest backup.\n\n"
      "\t-1\n"
      "\t    Limits --details to the newest backup for each set.\n\n"
      "Options:\n"
      "\t-n <name>, --name=<name>\n"
      "\t    Specifies the backup set's name. The name will be reflected as the index\n"
      "\t    file's name prefix.\n\n");
  } else if (op == "show") {
    printf("usage: shaback show [<general_options>] [-p <pw> | --password=<pw>] <id>\n\n");
    printf("\tDecompresses and decrypts the specified object from the repository to stdout.\n\n");
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
#if defined(OPENSSL_FOUND)
      "\t-E <enc>, --encryption=<enc>\n"
      "\t    Enable encryption for this repository. You cannot alter this setting\n"
      "\t    after the repository has been created!\n"
      "\t    Be sure to specify a password via the --password option (see below).\n"
      "\t    <enc> must be one of: `None', `Blowfish'. Defaults to `None'.\n\n"
#endif
      "\t-C <comp>, --compression=<comp>\n"
      "\t    Enable data compression for this repository. You cannot alter this setting\n"
      "\t    after the repository has been created!\n"
      "\t    <comp> must be one of: `None',\n"
      "\t    `BZip', `BZip-1', `BZip-9'"
#if defined(ZSTD_FOUND)
           ",\n\t    `ZStd', `ZStd-1', `ZStd-5', `ZStd-9'"
#endif
#if defined(LZMA_FOUND)
           ",\n\t    `LZMA', `LZMA-0', `LZMA-5', `LZMA-9'"
#endif
           "\n\t    or `Deflate'.\n"
      "\t    Defaults to `Deflate'.\n\n"
      "\t-F <fmt>, --repo-format=<fmt>\n"
      "\t    Select an alternative repository format.\n"
      "\t    <fmt> must be one of: `2-2' or `3', where `2-2' is the default.\n\n"
      "\t-p <pw>, --password=<pw>\n"
      "\t    If encryption is enabled, this specifies the password to be used.\n\n");
  } else if (op == "deflate") {
    printf("usage: shaback deflate\n\n");
  } else if (op == "inflate") {
    printf("usage: shaback inflate\n\n");
  } else {
    printf("usage: shaback <command> [<options>] [<args>]\n");
    printf("\n");
    printf("Valid commands are:\n");
    printf("   backup        Backup a set of files or directories.\n");
    printf("   gc            Garbage collection: Delete unused files from archive.\n");
    printf("   prune         Remove index files according to deletion rules.\n");
    printf("   history       View / maintain backup history.\n");
    printf("   init          Create a new repository.\n");
    printf("   restore       Restore files from repository.\n");
    printf("   test-restore  Pretend to restore files, check hash digests and dump file listing.\n");
    printf("   list-files    Lists all files needed from the repo to restore a certain backup set.\n");
    printf("   show          Decompress and decrypt a certain object from the repository.\n");
    printf("   deflate       Compress data from stdin to stdout using `Deflate' compression.\n");
    printf("   inflate       Decompress data from stdin to stdout using `Deflate' compression.\n");
    printf("   migrate       Upgrade repository to version " SHABACK_REPO_VERSION ".\n");
    printf("   version       Print version information.\n");
    printf("\n");
    printf("General options are:\n");
    printf("\t-c <file>, --config=<file>\n"
        "\t    Additionally load specified config file.\n\n");
    printf("\t-i <error>, --ignore-error=<error>\n"
        "\t    Ignore errors of specified type. One of:\n"
        "\t    `chown', `chmod', `utime', 'invalid-filename'.\n\n");
    printf("\t-r <dir>, --repository=<dir>\n"
      "\t    Override repository specified in config files.\n\n");
    printf("\t-v, --verbose\n"
      "\t    Be verbose.\n\n");
    printf("\t-d, --debug\n"
      "\t    Be even more verbose.\n\n");
    printf("\t-h, --help\n"
      "\t     Show usage information.\n\n");
    printf("See `shaback <command> --help' for more information on a specific command.\n");
  }
}


static Repository* globalRepo;

static void interruptHandler(int sig)
{
  cerr << "Operation cancelled." << endl;
  globalRepo->unlock(true);
  exit(sig);
}

int main(int argc, char** argv)
{
  try {
    RuntimeConfig config;

    try {
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
        globalRepo = &shaback.repository;

        signal(SIGINT, interruptHandler);
        signal(SIGTERM, interruptHandler);
        signal(SIGHUP, interruptHandler);
        signal(SIGPIPE, interruptHandler);

        if (config.operation == "init") {
          shaback.createRepository();
        } else if (config.operation == "backup") {
          return shaback.repository.backup();
        } else if (config.operation == "restore") {
          RestoreReport report = shaback.repository.restore();
          return report.hasErrors() ? 1 : 0;
        } else if (config.operation == "test-restore") {
          RestoreReport report(shaback.repository.testRestore());
          return report.hasErrors() ? 1 : 0;
        } else if (config.operation == "list-files") {
          RestoreReport report(shaback.repository.listFiles());
          return report.hasErrors() ? 1 : 0;
        } else if (config.operation == "show") {
          shaback.repository.show();
        } else if (config.operation == "gc") {
          shaback.repository.gc();
        } else if (config.operation == "prune") {
          shaback.repository.prune();
        } else if (config.operation == "history") {
          shaback.repository.history();
        } else if (config.operation == "migrate") {
          shaback.repository.migrate();
        } else if (config.operation == "deflate") {
          return shaback.deflate();
        } else if (config.operation == "inflate") {
          return shaback.inflate();
        } else if (config.operation == "version") {
          printf("Shaback version %u.%u\n", SHABACK_VERSION_MAJOR, SHABACK_VERSION_MINOR);
        } else {
          cerr << "Invalid operation `" << config.operation << "'." << endl;
          return 1;
        }
        return 0;
      }
    } catch (Exception& ex) {
      cerr << config.color_error << ex.getMessage() << config.color_default << endl;
      return 10;
    }
  } catch (Exception& ex) {
    cerr << ex.getMessage() << endl;
//    config.runErrorCallbacks(ex.getMessage());
    return 10;
  }
}

