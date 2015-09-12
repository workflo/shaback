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

#include "Properties.h"
#include "Exception.h"
#include "BufferedReader.h"
#include "FileInputStream.h"

using namespace std;

bool isWhitespace(char ch)
{
  return (ch == ' ' || ch == '\t');
}

/*****************************************************************************\
 * Properties                                                                 |
 *****************************************************************************/
Properties::Properties()
{
}

/*****************************************************************************\
 * ~Properties                                                                |
 *****************************************************************************/
Properties::~Properties()
{
}

/*****************************************************************************\
 * load                                                                       |
 *****************************************************************************/
void Properties::load(InputStream& in)
{
  string line;

  while (in.readLine(line)) {
    char c = 0;
    size_t pos = 0;

    // If empty line or begins with a comment character, skip this line.
    if (line.empty() || line.at(0) == '#' || line.at(0) == '!')
      continue;

    while (pos < line.size() && ::isWhitespace(c = line.at(pos)))
      pos++;

    // If line is empty skip this line.
    if (pos == line.size())
      continue;

    // The characters up to the next Whitespace, ':', or '='
    // describe the key.  But look for escape sequences.
    string key;
    while (pos < line.size() && !::isWhitespace(c = line.at(pos++)) && c != '=' && c != ':') {
      if (c == '\\') {
        if (pos == line.size()) {
          // The line continues on the next line.
          in.readLine(line);
          pos = 0;
          while (pos < line.size() && ::isWhitespace(c = line.at(pos)))
            pos++;
        } else {
          c = line.at(pos++);
          switch (c) {
            case 'n':
              key.append("\n");
              break;
            case 't':
              key.append("\t");
              break;
            case 'r':
              key.append("\r");
              break;
              //            case 'u':
              //              if (pos + 4 <= line.size()) {
              //                char uni = (char) Integer::parseInt(line->substring(pos, pos + 4), 16);
              //                key.append(uni);
              //                pos += 4;
              //              } // else throw exception?
              //              break;
            default:
              key.append(&c, 1);
              break;
          }
        }
      } else {
        key.append(&c, 1);
      }
    }

    bool isDelim = (c == ':' || c == '=');
    while (pos < line.size() && ::isWhitespace(c = line.at(pos)))
      pos++;

    if (!isDelim && (c == ':' || c == '=')) {
      pos++;
      while (pos < line.size() && ::isWhitespace(c = line.at(pos)))
        pos++;
    }

    string element;
    while (pos < line.size()) {
      c = line.at(pos++);
      if (c == '\\') {
        if (pos == line.size()) {
          // The line continues on the next line.
          if (!in.readLine(line))
            break;

          pos = 0;
          while (pos < line.size() && ::isWhitespace(c = line.at(pos)))
            pos++;
        } else {
          c = line.at(pos++);
          switch (c) {
            case 'n':
              element.append("\n");
              break;
            case 't':
              element.append("\t");
              break;
            case 'r':
              element.append("\r");
              break;
              //            case 'u':
              //              if (pos + 4 <= line.size()) {
              //                char uni = (char) Integer::parseInt(line->substring(pos, pos + 4), 16);
              //                element.append(uni);
              //                pos += 4;
              //              } // else throw exception?
              //              break;
            default:
              element.append(&c, 1);
              break;
          }
        }
      } else {
        element.append(&c, 1);
      }
    }
    map[key] = element;
  }
}

void Properties::load(File& f)
{
  FileInputStream in(f);
  BufferedReader reader(&in);
  load(reader);
}

std::string Properties::getProperty(string& key)
{
  return map[key];
}

std::string Properties::getProperty(const char* key)
{
  return map[string(key)];
}
