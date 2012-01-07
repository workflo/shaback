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

#ifndef SHABACK_Properties_H
#define SHABACK_Properties_H

#include <string>
#include <map>
#include "InputStream.h"
#include "File.h"

class Properties
{
  public:
    Properties();
    void load(InputStream& in);
    void load(File& f);
    std::string getProperty(std::string& key);
    std::string getProperty(const char* key);

    virtual ~Properties();

  private:
    std::map<std::string, std::string> map;
};
#endif // SHABACK_Properties_H
