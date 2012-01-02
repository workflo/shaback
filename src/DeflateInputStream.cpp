#include <iostream>

#include "DeflateInputStream.h"
#include "Exception.h"

using namespace std;


DeflateInputStream::DeflateInputStream(InputStream* in)
  : in(in)
{
  ret = Z_OK;
  zipStream.zalloc = Z_NULL;
  zipStream.zfree = Z_NULL;
  zipStream.opaque = Z_NULL;

//  inputBufferCapacity = DEFLATE_CHUNK_SIZE * 2;
//  inputBufferBytesAvailable = 0;
//  inputBuffer = (unsigned char*) malloc(inputBufferCapacity);

  if (inflateInit(&zipStream) != Z_OK) {
    // TODO: Throw exception
    cout << "DeflateInputStream error" << endl;
  }
}

DeflateInputStream::~DeflateInputStream()
{
//  free(inputBuffer);
  close();
}


int DeflateInputStream::read()
{
  throw UnsupportedOperation("read()");
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
  if (zipStream.avail_in == 0) {
    int bytesRead = in->read((char*) readBuffer, min(len, DEFLATE_CHUNK_SIZE));

    zipStream.avail_in = bytesRead;
    zipStream.next_in = readBuffer;

    if (bytesRead == -1) return -1;
  }
    
  zipStream.avail_out = len;
  zipStream.next_out = (unsigned char*) b;

//  cerr << "DeflateInputStream.read: avail_in=" << zipStream.avail_in << "; avail_out=" << zipStream.avail_out << endl;
      
  ret = inflate(&zipStream, Z_NO_FLUSH);
//  cerr << "   -> avail_in=" << zipStream.avail_in << "; avail_out=" << zipStream.avail_out << "; written=" << (DEFLATE_CHUNK_SIZE - zipStream.avail_out) << "; ret=" << ret << endl;

//  if (zipStream.avail_in > 0) {
//	  cerr << "     more to read: " << zipStream.avail_in << endl;
//  }

  if (ret == Z_STREAM_END && zipStream.avail_out == len) {
    return -1;
  } else if (ret == Z_STREAM_ERROR) {
    // TODO: Throw exception
  }

//  cerr << "Bytes returned: "<<(DEFLATE_CHUNK_SIZE - zipStream.avail_out) << endl;

  return DEFLATE_CHUNK_SIZE - zipStream.avail_out;
}


void DeflateInputStream::close()
{
  if (ret != Z_STREAM_END) {
    inflateEnd(&zipStream);
    in->close();
    ret = Z_STREAM_END;
  }
}

