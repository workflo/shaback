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
#include "Sha1.h"

using namespace std;

Sha1::Sha1()
{
  ctx = (SHA_CTX*) malloc(sizeof(SHA_CTX));
  reset();
}

Sha1::~Sha1()
{
  free(ctx);
  ctx = 0;
}

void Sha1::reset()
{
  SHA1_Init(ctx);
}

void Sha1::update(const void* data, unsigned long len)
{
  SHA1_Update(ctx, data, len);
}

void Sha1::update(std::string& data)
{
  SHA1_Update(ctx, data.data(), data.size());
}

string Sha1::toString()
{
  return hexStr;
}

const unsigned char* Sha1::toBytes()
{
  return bytes;
}

void Sha1::finalize()
{
  char hexChars[SHA_DIGEST_LENGTH * 2 + 1];

  SHA1_Final(bytes, ctx);

  for (int x = 0; x < SHA_DIGEST_LENGTH; x++) {
    hexChars[x * 2] = HEX_CHARS[bytes[x] >> 4];
    hexChars[x * 2 + 1] = HEX_CHARS[bytes[x] & 0x0f];
  }
  hexChars[SHA_DIGEST_LENGTH * 2] = 0;

  hexStr = hexChars;
}
