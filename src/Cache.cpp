#include <iostream>

#include "Cache.h"
#include "Exception.h"


using namespace std;

Cache::Cache(File file)
: file(file), opened(false)
{
}

Cache::~Cache()
{
    close();
}


void Cache::open(int openMode)
{
  if (!opened) {
    gdbmFile = gdbm_open((char*) file.path.c_str(), 4096, openMode, 0777, 0);
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


void Cache::close()
{
  if (opened) {
    gdbm_close(gdbmFile);
    opened = false;
  }
}


bool Cache::contains(string& key)
{
  if (opened) {
    datum k;
    k.dptr = (char*) key.data();
    k.dsize = key.length();
    bool found = gdbm_exists(gdbmFile, k);
    return found;
  } else {
    return false;
  }
}


void Cache::put(string& key, string& value)
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


void Cache::put(string& key)
{
  char empty;

  if (opened) {
    datum k;
    k.dptr = (char*) key.data();
    k.dsize = key.length();
    datum v;
    v.dptr = &empty;
    v.dsize = 0;
    gdbm_store(gdbmFile, k, v, GDBM_REPLACE);
  }  
}


void Cache::remove(string& key)
{
  if (opened) {
    datum k;
    k.dptr = (char*) key.data();
    k.dsize = key.length();
    gdbm_delete(gdbmFile, k);  
  }  
}


void Cache::exportCache(OutputStream& out)
{
  if (opened) {
	datum key = gdbm_firstkey (gdbmFile);
	while (key.dptr) {
	  out.write(key.dptr, key.dsize);
	  out.write("\n", 1);

	  datum nextkey = gdbm_nextkey (gdbmFile, key);
      free (key.dptr);
      key = nextkey;
    }
  }
}


void Cache::importCache(InputStream& in)
{
  if (opened) {
    string str;
    while(in.readLine(str)) {
      put(str);
    }
  }
}
