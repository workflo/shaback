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
#include <stdlib.h>
#include <string.h>

#include "Exception.h"
#include "KeyDerivation.h"

#if defined(OPENSSL_FOUND)

#include <openssl/evp.h>

unsigned char* KeyDerivation::deriveFromPassword(std::string& password)
{
    unsigned char* key = (unsigned char*) malloc(EVP_MAX_KEY_LENGTH);
    int iter;
    int pwLen = password.size();

    if (pwLen < 8) {
      throw Exception("Crypto Password must be at least 8 characters.");
    }

    iter = 10000000 / pwLen;

    if (!PKCS5_PBKDF2_HMAC_SHA1( (const char*) password.c_str(), pwLen, SHABACK_KEY_SALT, strlen((const char*) SHABACK_KEY_SALT), iter, EVP_MAX_KEY_LENGTH, key)) {
      throw Exception("PKCS5_PBKDF2_HMAC_SHA1 failed.");
    }

    return key;
}

#endif
