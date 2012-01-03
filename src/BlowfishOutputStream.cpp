#include <iostream>

#include "BlowfishOutputStream.h"
#include "Exception.h"

using namespace std;

BlowfishOutputStream::BlowfishOutputStream(string& password, OutputStream* out) :
  out(out)
{
  strncpy((char*) iv, "SHABACK2", BF_BLOCK);

  memset(key, 0, 16);
  strncpy((char*) key, password.data(), min(16, (int) password.size()));

  EVP_CIPHER_CTX_init(&ctx);
  EVP_EncryptInit_ex(&ctx, EVP_bf_cbc(), NULL, key, iv);

  outlen = 0;
}

BlowfishOutputStream::~BlowfishOutputStream()
{
  close();
  EVP_CIPHER_CTX_cleanup(&ctx);
}

void BlowfishOutputStream::write(int b)
{
  char c = (char) b;
  write(&c, 1);
}

void BlowfishOutputStream::write(const char* b, int len)
{
  if (len <= 0)
    return;

  if (!EVP_EncryptUpdate(&ctx, outputBuffer, &outlen, (const unsigned char*) b, len)) {
    // TODO: Error
  }

  out->write((const char*) outputBuffer, outlen);
}

void BlowfishOutputStream::finish()
{
  if (!EVP_CipherFinal_ex(&ctx, outputBuffer, &outlen)) {
    // TODO: Error
  }
  out->write((const char*) outputBuffer, outlen);
}

void BlowfishOutputStream::close()
{
  if (out) {
    finish();
    out->close();
    out = 0;
  }
}

