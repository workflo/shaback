/*
 * shaback - A hash digest based backup tool.
 * Copyright (C) 2015 Florian Wolff (florian@donuz.de)
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
#include "StopWatch.h"

using namespace std;

StopWatch::StopWatch(string name)
: name(name)
{
  counter = 0;
  milliSeconds = 0;
}


long long StopWatch::currentTimeMillis()
{
  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv, &tz);

  return ((long long) tv.tv_sec) * 1000LL + tv.tv_usec / 1000LL;
}


void StopWatch::start()
{
  startTime = currentTimeMillis();
}


void StopWatch::stop()
{
  counter++;
  milliSeconds += currentTimeMillis() - startTime;
}


int StopWatch::getCount()
{
  return counter;
}

long long StopWatch::getTotalMillis()
{
  return milliSeconds;
}


double StopWatch::getAveragMillis()
{
  return (counter > 0 ? milliSeconds / counter : 0);
}


string StopWatch::getName()
{
  return name;
}


string StopWatch::toString()
{
  char s[200];
  sprintf(s, "%20s: n=%6d; avg=%03.3f", name.c_str(), counter, getAveragMillis() / 1000.0);
  return s;
}
