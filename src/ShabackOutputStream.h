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

#ifndef SHABACK_ShabackOutputStream_H
#define SHABACK_ShabackOutputStream_H

#include <string>
#include <zlib.h>
#include "lib/File.h"
#include "lib/OutputStream.h"
#include "lib/config.h"
#include "RuntimeConfig.h"

class ShabackOutputStream
{
  public:
    ShabackOutputStream(RuntimeConfig& config, int compressionAlgorithm, int encryptionAlgorithm);
    ~ShabackOutputStream();

    void open(File& file);
    void close();
    void finish();

    void write(std::string& s);
    void write(const char* s);
    void write(const char* data, int numBytes);

    static OutputStream* createCompressionStream(OutputStream* outputStream, int compressionAlgorithm);

  private:
    int compressionAlgorithm;
    int encryptionAlgorithm;
    File file;
    File tmpFile;
    bool opened;
    OutputStream* compressionOutputStream;
    OutputStream* outputStream;
    OutputStream* fileOutputStream;
    OutputStream* encryptionOutputStream;
    RuntimeConfig& config;
};

#endif // SHABACK_ShabackOutputStream_H
