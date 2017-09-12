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

#ifndef SHABACK_RestoreReport_H
#define SHABACK_RestoreReport_H

#include <stdint.h>


class RestoreReport
{
  public:
    RestoreReport();
    RestoreReport(const RestoreReport& report);
    RestoreReport& operator=(const RestoreReport& report);

    bool inline hasErrors() { return numErrors > 0; }
    void dump();

    intmax_t numBytesRestored;
    intmax_t bytesToBeRestored;
    int numFilesRestored;
    int numErrors;
    unsigned int fileCount;
};

#endif // SHABACK_RestoreReport_H
