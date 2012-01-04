#include <iostream>

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
