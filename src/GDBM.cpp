#include <iostream>

#include "GDBM.h"
#include "Exception.h"


using namespace std;

GDBM::GDBM(File file)
: file(file), opened(false)
{
}

GDBM::~GDBM()
{
    close();
}


void GDBM::open()
{
  if (!opened) {
    //char path[MAX_PATH_LEN];
    //file.path.copy(path, MAX_PATH_LEN);
    //gdbmFile = gdbm_open(path, 4096, GDBM_WRCREAT, 0777, 0);
    gdbmFile = gdbm_open((char*) file.path.c_str(), 4096, GDBM_WRCREAT, 0777, 0);
    if (gdbmFile == 0) {
      if (errno > 0) {
	throw Exception::errnoToException(file.path);
      } else {
	// TODO: Fehler
      }
    } else {
      opened = true;
    }
  }
}


void GDBM::close()
{
  if (opened) {
    cout << "Closing GDBM: " << file.path << endl;
    gdbm_close(gdbmFile);
    opened = false;
  }
}


bool GDBM::contains(string& key)
{
  if (opened) {
    datum k;
    k.dptr = (char*) key.data();
    k.dsize = key.length();
//    if (gdbm_exists(gdbmFile, k)) {
//      cout << "GDBM contains " << key << endl;
//    }
    return gdbm_exists(gdbmFile, k);
  } else {
    return false;
  }
}


void GDBM::put(string& key, string& value)
{
  if (opened) {
    datum k;
    k.dptr = (char*) key.data();
    k.dsize = key.length();
    datum v;
    v.dptr = (char*) value.data();
    v.dsize = value.length();
    gdbm_store(gdbmFile, k, v, GDBM_REPLACE);  
  }  
}


void GDBM::put(string& key)
{
  if (opened) {
    datum k;
    k.dptr = (char*) key.data();
    k.dsize = key.length();
    datum v;
    v.dptr = "";
    v.dsize = 0;
    gdbm_store(gdbmFile, k, v, GDBM_REPLACE);  
  }  
}


void GDBM::remove(string& key)
{
  if (opened) {
    datum k;
    k.dptr = (char*) key.data();
    k.dsize = key.length();
    gdbm_delete(gdbmFile, k);  
  }  
}
