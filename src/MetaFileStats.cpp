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
 
 
#include "MetaFileStats.h"
 
MetaFileStats::MetaFileStats()
{
  reset();
}


void MetaFileStats::dump()
{
  if (treeFilesRead > 0) {
    #ifdef __APPLE__
    fprintf(stderr, "Meta data bytes read: %12jd\n", treeFileBytesRead);
    #else
    fprintf(stderr, "Meta data bytes read: %12jd\n", treeFileBytesRead);
    #endif
    fprintf(stderr, "Meta data files read: %12d\n", treeFilesRead);
  }
}


void MetaFileStats::reset()
{
  treeFilesRead = 0;
  treeFileBytesRead = 0;
}
