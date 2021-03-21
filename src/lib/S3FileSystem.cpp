/*
 * shaback - A hash digest based backup tool.
 * Copyright (C) 2021 Florian Wolff (florian@donuz.de)
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

#include "S3FileSystem.h"
#include "S3File.h"
#include <iostream>
#include "Exception.h"

using namespace std;

S3FileSystem::S3FileSystem(string url) {
  if (url.find_first_of("s3://") == 0) {
    bucket = url.substr(5, string::npos);

    auto slashPos = bucket.find_first_of('/');
    if (slashPos != string::npos) {
      rootDir = bucket.substr(slashPos, string::npos);
      bucket = bucket.substr(0, slashPos);

      while (rootDir.back() == '/') {
        rootDir.pop_back();
      }
    } else {
      rootDir = '/';
    }

    cout << "S3FileSystem::S3FileSystem bucket=" << bucket << "; rootDir=" << rootDir << "\n";
  } else {
    throw InvalidUrlException(string("Invalid S3 URL: ").append(url));
  }
}

File S3FileSystem::file(std::string path) {
  cout << "   S3File::file(" << path << ")\n";
  return S3File(this, path);
}

File S3FileSystem::file(File parent, std::string filename) {
  cout << "   S3File::file(" << parent.path << "/" << filename << ")\n";
  return S3File(this, parent, filename);
}
