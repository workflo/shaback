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

#include "Cache.h"
#include "lib/Exception.h"

using namespace std;

Cache::Cache(File file) :
    file(file), opened(false)
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
        // TODO: GDBM: handle error
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

string Cache::get(string& key)
{
  if (opened) {
    datum k;
    k.dptr = (char*) key.data();
    k.dsize = key.length();
    datum v = gdbm_fetch(gdbmFile, k);

    if (v.dptr) {
      string s(v.dptr, v.dsize);
      free(v.dptr);
      return s;
    }
  }

  return "";
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
