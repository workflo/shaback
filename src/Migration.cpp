/*
 * shaback - A hash digest based backup tool.
 * Copyright (C) 2017 Florian Wolff (florian@donuz.de)
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
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
   
#include "shaback.h"
#include "Migration.h"
#include "lib/Exception.h"

using namespace std;


Migration::Migration(RuntimeConfig& config, Repository& repository) :
    repository(repository), config(config)
{
}
 
void Migration::run()
{
  repository.open();

  if (repository.version == SHABACK_REPO_VERSION) {
    cerr << "No need to migrate repository." << endl;
  } else if (repository.version == "2") {
    repository.lock();
    cout << "Migrating repository from version \"" << repository.version << "\" to \"3\"..." << endl;
    repository.unlock();
  } else {
    throw Exception(string("Unsupported repository version \"").append(repository.version).append("\"."));
  }
}
