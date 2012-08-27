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

#ifndef SHABACK_Date_H
#define SHABACK_Date_H

#include <string>
#include <time.h>

class Date
{
  public:
    Date();
    Date(std::string str);
    inline int getYear() { return year; }
    inline int getMonth() { return month; }
    inline int getDay() { return day; }
    inline int getHour() { return hour; }
    inline int getMinute() { return minute; }
    inline int getSecond() { return second; }
    void addHours(int x);
    void addMinutes(int x);
    void addSeconds(int x);
    void addDays(int x);
    void addMonths(int x);
    void addYears(int x);

    std::string toFilename();

  private:
    time_t rawtime;
    int year, month, day, hour, minute, second;
    void internalize();
};

#endif // SHABACK_Date_H

