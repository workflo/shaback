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
#include <stdlib.h>
#include "Date.h"

using namespace std;

Date::Date()
{
  time(&rawtime);
  internalize();
}

Date::Date(string str)
{
  year = strtol(str.substr(0, 4).c_str(), 0, 10);
  month = strtol(str.substr(5, 2).c_str(), 0, 10);
  day = strtol(str.substr(8, 2).c_str(), 0, 10);
  hour = strtol(str.substr(11, 2).c_str(), 0, 10);
  minute = strtol(str.substr(13, 2).c_str(), 0, 10);
  second = strtol(str.substr(15, 2).c_str(), 0, 10);

  struct tm ptm;
  ptm.tm_isdst = 0;
  ptm.tm_zone = 0;
  ptm.tm_gmtoff = 0;
  ptm.tm_year = year - 1900;
  ptm.tm_mon = month -1;
  ptm.tm_mday = day;
  ptm.tm_hour = hour;
  ptm.tm_min = minute;
  ptm.tm_sec = second;

  // If timegm should not be available: http://linux.die.net/man/3/timegm
  rawtime = timegm(&ptm);
  internalize();
}

string Date::toFilename()
{
  char filename[100];
  sprintf(filename, "%04d-%02d-%02d_%02d%02d%02d", getYear(), getMonth(), getDay(), getHour(), getMinute(), getSecond());

  return filename;
}

string Date::toString()
{
  return ctime(&rawtime);
}

void Date::addYears(int x)
{
  struct tm * ptm;
  ptm = gmtime(&rawtime);

  ptm->tm_year += x;

  rawtime = timegm(ptm);
  internalize();
}

void Date::addMonths(int x)
{
  struct tm * ptm;
  ptm = gmtime(&rawtime);

  ptm->tm_mon += x;

  rawtime = timegm(ptm);
  internalize();
}

void Date::addDays(int x)
{
  struct tm * ptm;
  ptm = gmtime(&rawtime);

  ptm->tm_mday += x;

  rawtime = timegm(ptm);
  internalize();
}

void Date::addHours(int x)
{
  rawtime += x * 60LL * 60LL;
  internalize();
}

void Date::addMinutes(int x)
{
  rawtime += x * 60LL;
  internalize();
}

void Date::internalize()
{
  struct tm * ptm;
  ptm = gmtime(&rawtime);

  year = ptm->tm_year + 1900;
  month = ptm->tm_mon + 1;
  day = ptm->tm_mday;
  hour = ptm->tm_hour;
  minute = ptm->tm_min;
  second = ptm->tm_sec;
}

int Date::compareTo(Date other)
{
  if (rawtime > other.rawtime) return 1;
  else if (rawtime < other.rawtime) return -1;
  else return 0;
}

double Date::diff(Date other)
{
  return difftime(rawtime, other.rawtime) / (60 * 60 * 24);
}

void Date::setTimeOfDay(int h, int m, int s)
{
  struct tm * ptm;
  ptm = gmtime(&rawtime);

  ptm->tm_hour = h;
  ptm->tm_min = m;
  ptm->tm_sec = s;

  rawtime = timegm(ptm);
  internalize();
}
