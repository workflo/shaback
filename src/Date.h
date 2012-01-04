#ifndef SHABACK_Date_H
#define SHABACK_Date_H

#include <string>
#include <time.h>

class Date
{
  public:
    Date();
    inline int getYear() { return year; }
    inline int getMonth() { return month; }
    inline int getDay() { return day; }
    inline int getHour() { return hour; }
    inline int getMinute() { return minute; }
    inline int getSecond() { return second; }

  private:
    time_t rawtime;
    int year, month, day, hour, minute, second;
};

#endif // SHABACK_Date_H

