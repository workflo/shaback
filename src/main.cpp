#include <iostream>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "shaback.h"
#include "RuntimeConfig.h"
#include "Sha1.h"

using namespace std;


void showUsage()
{
  std::cout << "Usage..." << std::endl;
}


int main(int argc, char** argv)
{
    RuntimeConfig config;
    config.load();
    config.parseCommandlineArgs(argc, argv);
    config.finalize();
    
    if (config.operation.empty()) {
        showUsage();
        return 1;
    }
    
//    cout << "Exclude patterns: ";
//    for (vector<string>::iterator it = config.excludePatterns.begin(); it < config.excludePatterns.end(); it++ )
//        cout << " " << *it;
//    cout << endl;

    Shaback shaback(config);
    
    if (config.operation == "init") {
	shaback.createRepository();
    } else if (config.operation == "backup") {
      return shaback.repository.backup();
    } else {
      shaback.repository.open();
    }
}

