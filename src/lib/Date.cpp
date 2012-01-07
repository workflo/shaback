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
#include <stdio.h>

#include "Date.h"

using namespace std;

Date::Date()
{
  struct tm * ptm;
  time(&rawtime);
  ptm = gmtime(&rawtime);

  year = ptm->tm_year + 1900;
  month = ptm->tm_mon + 1;
  day = ptm->tm_mday;
  hour = ptm->tm_hour;
  minute = ptm->tm_min;
  second = ptm->tm_sec;
}

string Date::toFilename()
{
  char filename[100];
  sprintf(filename, "%04d-%02d-%02d_%02d%02d%02d", getYear(), getMonth(), getDay(), getHour(), getMinute(), getSecond());

  return filename;
}
