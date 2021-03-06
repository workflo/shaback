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

#ifndef SHABACK_ShabackException_H
#define SHABACK_ShabackException_H

#include "lib/Exception.h"

class RestoreException : public Exception
{
  public:
    RestoreException(std::string msg);
};

class InvalidTreeFile : public Exception
{
  public:
    InvalidTreeFile(std::string msg);
};

class GarbageCollectionException : public Exception
{
  public:
    GarbageCollectionException(std::string msg);
};

class PasswordException : public Exception
{
  public:
    PasswordException(std::string msg);
};

class LockingException : public Exception
{
  public:
    LockingException(std::string msg);
};
#endif // SHABACK_ShabackException_H
