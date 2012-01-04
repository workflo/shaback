#include <iostream>
#include <stdlib.h>
#include "Digest.h"

using namespace std;

char Digest::HEX_CHARS[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

bool Digest::looksLikeDigest(string& str)
{
  int len = str.size();
  if (len < 20) {
    // Too short:
    return false;
  }

  for (int x = 0; x < len; x++) {
    char c = str[x];
    if (! ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')))
      return false;
  }

  return true;
}
