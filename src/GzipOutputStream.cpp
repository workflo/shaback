#include <iostream>

#include "GzipOutputStream.h"
#include "Exception.h"

using namespace std;


GzipOutputStream::GzipOutputStream(OutputStream* out)
  : out(out)
{
  ret = 0;
  zipStream.zalloc = Z_NULL;
  zipStream.zfree = Z_NULL;
  zipStream.opaque = Z_NULL;
  if (deflateInit(&zipStream, 5) != Z_OK) {
    // TODO: Throw exception
    cout << "GzipOutputStream error" << endl;
  }

//   cout << "GzipOutputStream.init" << endl;
}

GzipOutputStream::~GzipOutputStream()
{
//   cout << "GzipOutputStream.~" << endl;
  close();
//   deflateEnd(&zipStream);
}


void GzipOutputStream::write(int b)
{
  char c = (char) b;
  write(&c, 1);
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

void GzipOutputStream::write(const char* b, int len)
{
  if (len <= 0) {
    return;
  } else if (len <= GZIP_CHUNK_SIZE) {
    zipStream.avail_in = len;
    zipStream.next_in = (unsigned char*) b;
    
    do {
      zipStream.avail_out = GZIP_CHUNK_SIZE;
      zipStream.next_out = outputBuffer;

//       cout << "GzipOutputStream.write: avail_in=" << zipStream.avail_in << "; avail_out=" << zipStream.avail_out << endl;
      
      ret = deflate(&zipStream, Z_NO_FLUSH);
//       cout << "   -> avail_in=" << zipStream.avail_in << "; avail_out=" << zipStream.avail_out << "; written=" << (GZIP_CHUNK_SIZE - zipStream.avail_out) << "; ret=" << ret<<endl;
      if (ret == Z_STREAM_ERROR) {
	// TODO: Throw exception
      }
      out->write((const char*) outputBuffer, GZIP_CHUNK_SIZE - zipStream.avail_out);
    } while (zipStream.avail_in > 0);

  } else {
    throw "Not yet implemented";
  }
}


void GzipOutputStream::finish()
{
  unsigned char inBuf[10];

//   cout << "GzipOutputStream.finish" << endl;
  zipStream.avail_in = 0;
  zipStream.next_in = inBuf;
    
  do {
    zipStream.avail_out = GZIP_CHUNK_SIZE;
    zipStream.next_out = outputBuffer;

//     cout << "GzipOutputStream.finish: avail_in=" << zipStream.avail_in << "; avail_out=" << zipStream.avail_out << endl;
      
    ret = deflate(&zipStream, Z_FINISH);
//     cout << "   -> avail_in=" << zipStream.avail_in << "; avail_out=" << zipStream.avail_out << "; written=" << (GZIP_CHUNK_SIZE - zipStream.avail_out) <<  "; ret=" << ret<<endl;
    if (ret == Z_STREAM_ERROR) {
      // TODO: Throw exception
    }
    out->write((const char*) outputBuffer, GZIP_CHUNK_SIZE - zipStream.avail_out);
  } while (ret != Z_STREAM_END);
}


void GzipOutputStream::close()
{
//   cout << "GzipOutputStream.close" << endl;
  if (ret != Z_STREAM_END) {
    finish();
    out->close();
  }
}

