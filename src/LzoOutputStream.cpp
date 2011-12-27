#include <iostream>
#include <stdlib.h>

#include "LzoOutputStream.h"
#include "Exception.h"

using namespace std;

// 16 KB output buffer, see http://content.gpwiki.org/index.php/LZO
#define COMPRESSION_CHUNK_SIZE (16 * 1024)

LzoOutputStream::LzoOutputStream(OutputStream* out)
  : out(out)
{
  lzo_init();
  workingMemory = (unsigned char*) malloc(LZO1X_1_MEM_COMPRESS);
  outputData = (unsigned char*) malloc(COMPRESSION_CHUNK_SIZE * 2);
}

LzoOutputStream::~LzoOutputStream()
{
  close();
  free(workingMemory);
  free(outputData);
}


void LzoOutputStream::write(int b)
{
  char c = (char) b;
  write(&c, 1);
}


void LzoOutputStream::write(const char* b, int bytesToCompress)
{
  lzo_uint dstLen;
  int inputOffset = 0;

  while (bytesToCompress > 0) {
    lzo1x_1_compress((const unsigned char*) &b[inputOffset], min(bytesToCompress, COMPRESSION_CHUNK_SIZE), outputData, &dstLen, workingMemory);
    cout << "LzoOutputStream::write: bytesToCompress=" << bytesToCompress << "; dstLen=" << dstLen << endl;
    inputOffset += COMPRESSION_CHUNK_SIZE;
    bytesToCompress -= COMPRESSION_CHUNK_SIZE;
    out->write((const char*) outputData, dstLen);
  }
}


void LzoOutputStream::finish()
{
}


void LzoOutputStream::close()
{
  finish();
  out->close();
}

