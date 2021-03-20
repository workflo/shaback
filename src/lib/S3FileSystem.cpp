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

using namespace std;

S3FileSystem::S3FileSystem(std::string url) {

}

File S3FileSystem::file(std::string path) {
    return File(path);
}

File S3FileSystem::file(File parent, std::string filename) {
    return File(parent, filename);
}

File S3FileSystem::home() {
    return File::home();
}

File S3FileSystem::tmpdir() {
    return File::tmpdir();
}
