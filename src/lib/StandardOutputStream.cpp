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

#include <fcntl.h>

#include "StandardOutputStream.h"
#include "Exception.h"

using namespace std;


StandardOutputStream::StandardOutputStream(FILE* handle)
  : handle(handle)
{
}

StandardOutputStream::~StandardOutputStream()
{
  close();
}


/*****************************************************************************\
 * write                                                                      |
 *****************************************************************************/
void StandardOutputStream::write(int b)
{
  char c = (char) b;
  write(&c, 1);
}


/*****************************************************************************\
 * write                                                                      |
 *****************************************************************************/
void StandardOutputStream::write(const char* b, int len)
{
  if (::fwrite(b, 1, len, handle) != len) {
    throw Exception::errnoToException();
  }
}


/*****************************************************************************\
 * close                                                                      |
 *****************************************************************************/
void StandardOutputStream::close() 
{
  flush();
}


void StandardOutputStream::flush()
{
  ::fflush(handle);
}

