#include <iostream>

#include "DeflateInputStream.h"
#include "Exception.h"

using namespace std;


DeflateInputStream::DeflateInputStream(InputStream* in)
  : in(in)
{
  ret = 0;
  zipStream.zalloc = Z_NULL;
  zipStream.zfree = Z_NULL;
  zipStream.opaque = Z_NULL;
  zipStream.avail_in = 0;
  zipStream.next_in = Z_NULL;

  if (inflateInit(&zipStream) != Z_OK) {
    // TODO: Throw exception
    cout << "DeflateInputStream error" << endl;
  }

//   cout << "DeflateInputStream.init" << endl;
}

DeflateInputStream::~DeflateInputStream()
{
//   cout << "DeflateInputStream.~" << endl;
  close();
//   deflateEnd(&zipStream);
}


int DeflateInputStream::read()
{
  char b;
  long r;
  r = read(&b, 1);
  if (r == 1)
    return b & 0xff;
  else 
    return -1;
}


// typedef struct z_stream_s {
//     Bytef    *next_in;  /* next input byte */
//     uInt     avail_in;  /* number of bytes available at next_in */
//     uLong    total_in;  /* total nb of input bytes read so far */

//     Bytef    *next_out; /* next output byte should be put there */
//     uInt     avail_out; /* remaining free space at next_out */
//     uLong    total_out; /* total nb of bytes output so far */

//     char     *msg;      /* last error message, NULL if no error */
//     struct internal_state FAR *state; /* not visible by applications */

//     alloc_func zalloc;  /* used to allocate the internal state */
//     free_func  zfree;   /* used to free the internal state */
//     voidpf     opaque;  /* private data object passed to zalloc and zfree */

//     int     data_type;  /* best guess about the data type: binary or text */
//     uLong   adler;      /* adler32 value of the uncompressed data */
//     uLong   reserved;   /* reserved for future use */
// } z_stream;

int DeflateInputStream::read(char* b, int len)
{
  if (len <= 0) return;

  int bytesRead = in->read(inputBuffer, min(len, DEFLATE_CHUNK_SIZE));

  zipStream.avail_in = bytesRead;
  zipStream.next_in = inputBuffer;
    
  do {
    zipStream.avail_out = len;
    zipStream.next_out = b;

//       cout << "DeflateInputStream.write: avail_in=" << zipStream.avail_in << "; avail_out=" << zipStream.avail_out << endl;
      
    ret = inflate(&zipStream, Z_NO_FLUSH);
//     cout << "   -> avail_in=" << zipStream.avail_in << "; avail_out=" << zipStream.avail_out << "; written=" << (GZIP_CHUNK_SIZE - zipStream.avail_out) << "; ret=" << ret<<endl;
    if (ret == Z_STREAM_ERROR) {
      // TODO: Throw exception
    }
    out->write((const char*) outputBuffer, DEFLATE_CHUNK_SIZE - zipStream.avail_out);
  } while (zipStream.avail_out == 0);
  //  } while (zipStream.avail_in > 0);
}


void DeflateInputStream::finish()
{
  unsigned char inBuf[10];

//   cout << "DeflateInputStream.finish" << endl;
  zipStream.avail_in = 0;
  zipStream.next_in = inBuf;
    
  do {
    zipStream.avail_out = DEFLATE_CHUNK_SIZE;
    zipStream.next_out = outputBuffer;

//     cout << "DeflateInputStream.finish: avail_in=" << zipStream.avail_in << "; avail_out=" << zipStream.avail_out << endl;
      
    ret = deflate(&zipStream, Z_FINISH);
//     cout << "   -> avail_in=" << zipStream.avail_in << "; avail_out=" << zipStream.avail_out << "; written=" << (GZIP_CHUNK_SIZE - zipStream.avail_out) <<  "; ret=" << ret<<endl;
    if (ret == Z_STREAM_ERROR) {
      // TODO: Throw exception
    }
    out->write((const char*) outputBuffer, DEFLATE_CHUNK_SIZE - zipStream.avail_out);
  } while (zipStream.avail_out == 0);

  inflateEnd(&zipStream);
}


void DeflateInputStream::close()
{
//   cout << "DeflateInputStream.close" << endl;
  if (ret != Z_STREAM_END) {
    finish();
    out->close();
  }
}

