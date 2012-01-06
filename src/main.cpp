#include <iostream>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "shaback.h"
#include "RuntimeConfig.h"
#include "lib/Sha1.h"
#include "lib/Exception.h"

using namespace std;

void showUsage(string& op)
{
  if (op == "backup") {
    printf("usage: shaback backup [<general_options>] [-n <name> | --name <name>]\n"
      "                      [-t | --totals] [-p <pw> | --password=<pw>]\n\n");
  } else if (op == "restore") {
    printf("usage: shaback restore [<general_options>] [-p <pw> | --password=<pw>] <treespec>\n\n");
    printf("\tRestores directories and files from the repository.\n"
      "\t<treespec> can be either a filename from the repository's index/ directory\n"
      "\tor the ID of the directory to be restored.\n"
      "\n"
      "\tFiles will always be restored into the CWD.\n");
  } else if (op == "show") {
    printf("usage: shaback show [<general_options>] [-p <pw> | --password=<pw>] <id>\n\n");
    printf("\tDecompresses and decrypts the specified object from the repository to stdout.\n\n");
  } else if (op == "init") {
    printf("usage: shaback init [<general_options>] [-f | --force]\n\n"
      "\tCreates a new repository at the location specified in one of the config files\n"
      "\tor via the --repository option. Defaults to the current working directory.\n\n"
      "Options:\n"
      "\t-f, --force\n"
      "\t    Force creation even if the destination directory is not empty.\n\n");
  } else if (op == "deflate") {
    printf("usage: shaback deflate\n\n");
  } else if (op == "inflate") {
    printf("usage: shaback inflate\n\n");
  } else {
    printf("usage: shaback <command> [<options>] [<args>]\n");
    printf("\n");
    printf("Valid commands are:\n");
    printf("   backup      Backup a set of files or directories.\n");
    printf("   gc          Garbage collection: Delete unused files from archive\n");
    //  printf("   extract     Extract backup sets from archive.\n");
    printf("   fsck        Perform integrity check with optional garbage collection.\n");
    printf("   init        Create a new repository.\n");
    printf("   restore     Restore files from repository.\n");
    printf("   cleanup     Delete old index files\n");
    printf("   show        Decompress and decrypt a certain object from the repository.\n");
    printf("   deflate     Compress data from stdin to stdout using `Deflate' compression.\n");
    printf("   inflate     Decompress data from stdin to stdout using `Deflate' compression.\n");
    printf("\n");
    printf("General options are:\n");
    printf("\t-c <file>, --config=<file>\n"
      "\t    Additionally load specified config file.\n\n");
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
        shaback.createRepository();
      } else if (config.operation == "backup") {
        return shaback.repository.backup();
      } else if (config.operation == "restore") {
        shaback.repository.restore();
        return 0;
      } else if (config.operation == "show") {
        shaback.repository.show();
        return 0;
      } else if (config.operation == "deflate") {
        return shaback.deflate();
      } else if (config.operation == "inflate") {
        return shaback.inflate();
      } else {
        cerr << "Invalid operation `" << config.operation << "'." << endl;
        return 1;
      }
    }
  } catch (Exception& ex) {
    cerr << ex.getMessage() << endl;
    return 10;
  }
}

