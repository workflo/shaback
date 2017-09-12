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
#include <stdlib.h>
#include <stdio.h>


#include "RestoreReport.h"

RestoreReport::RestoreReport() :
    numErrors(0), numFilesRestored(0), numBytesRestored(0), fileCount(0), bytesToBeRestored(0)
{}

RestoreReport::RestoreReport(const RestoreReport& orig) :
    numErrors(orig.numErrors), numFilesRestored(orig.numFilesRestored), numBytesRestored(orig.numBytesRestored),
    fileCount(orig.fileCount), bytesToBeRestored(orig.bytesToBeRestored)
{}


void RestoreReport::dump()
{
  fprintf(stderr, "Files restored:   %12d                      \n", numFilesRestored);
  #ifdef __APPLE__
  fprintf(stderr, "Bytes restored:   %12jd\n", numBytesRestored);
  #else
  fprintf(stderr, "Bytes restored:   %12jd\n", numBytesRestored);
  #endif
  fprintf(stderr, "Errors:           %12d\n", numErrors);
}
