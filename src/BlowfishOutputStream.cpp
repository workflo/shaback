#include <iostream>

#include "BlowfishOutputStream.h"
#include "Exception.h"
#include "Sha256.h"

using namespace std;

BlowfishOutputStream::BlowfishOutputStream(string& password, OutputStream* out) :
  out(out)
{
  // Static Initialization Vector:
  strncpy((char*) iv, SHABACK_IV, BF_BLOCK);

  // Compute SHA-256 digest of password:
  Sha256 sha;
  sha.update(password);
  sha.finalize();

  // Use first 16 bytes of digest as key:
  strncpy((char*) key, (const char*) sha.toBytes(), 16);

  EVP_CIPHER_CTX_init(&ctx);
  EVP_EncryptInit_ex(&ctx, EVP_bf_cbc(), NULL, key, iv);

  outlen = 0;
  //  remainderLen = 0;
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
    throw IOException("EVP_EncryptUpdate failed");
  }

  out->write((const char*) outputBuffer, outlen);

  //  cout << "write: len=" << len << "; outlen=" << outlen << endl;

  //  remainderLen = (len % BF_BLOCK);
  //  if (remainderLen) {
  //    cout << "Padding: " << remainderLen << endl;
  //  }
}

void BlowfishOutputStream::finish()
{
  if (!EVP_CipherFinal_ex(&ctx, outputBuffer, &outlen)) {
    throw IOException("EVP_CipherFinal_ex failed");
  }
  out->write((const char*) outputBuffer, outlen);
  //  cout << "finish: outlen=" << outlen << endl;
}

void BlowfishOutputStream::close()
{
  if (out) {
    finish();
    out->close();
    out = 0;
  }
}
