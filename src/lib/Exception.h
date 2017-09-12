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

#ifndef SHABACK_Exception_H
#define SHABACK_Exception_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <exception>
#include <errno.h>

class Exception
{
  public:
    Exception();
    Exception(std::string msg);
    virtual std::string getMessage();
    static Exception errnoToException();
    static Exception errnoToException(std::string filename);
    static Exception errnoToException(int e, std::string filename);

  protected:
    std::string msg;
};

class IOException: public Exception
{
  public:
    IOException(std::string msg);
};

class FileNotFoundException: public IOException
{
  public:
    FileNotFoundException(std::string filename);
    virtual std::string getFilename();

  protected:
    std::string filename;
};

class IllegalStateException : public Exception
{
  public:
    IllegalStateException(std::string msg);
};

class UnsupportedCompressionAlgorithm: public Exception
{
  public:
    UnsupportedCompressionAlgorithm(std::string algo);
};

class UnsupportedEncryptionAlgorithm: public Exception
{
  public:
    UnsupportedEncryptionAlgorithm(std::string algo);
};

class UnsupportedRepositoryFormat: public Exception
{
  public:
    UnsupportedRepositoryFormat(std::string algo);
};

class UnsupportedOperation: public Exception
{
  public:
    UnsupportedOperation(std::string op);
};

class MissingCryptoPassword: public Exception
{
  public:
    MissingCryptoPassword();
};

class DeflateException : public Exception
{
  public:
    DeflateException(std::string msg);
};

class BzException : public Exception
{
  public:
    BzException(std::string msg);
    BzException(std::string msg, int err);
};

class LzmaException : public Exception
{
  public:
    LzmaException(std::string msg);
    LzmaException(std::string msg, int err);
};

class ZStdException : public Exception
{
  public:
    ZStdException(std::string msg);
    ZStdException(std::string msg, int err);
    ZStdException(std::string msg, std::string details);
};

#endif // SHABACK_Exception_H
