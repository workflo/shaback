
#include "GzipOutputStream.h"
#include "Exception.h"

using namespace std;


GzipOutputStream::GzipOutputStream(OutputStream* out)
  : out(out)
{
  zipStream.zalloc = 0;
  zipStream.zfree = 0;
  zipStream.opaque = 0;
  if (deflateInit(&zipStream, 5) != Z_OK) {
    // TODO: Throw exception
  }
}

GzipOutputStream::~GzipOutputStream()
{
  close();
  deflateEnd(&zipStream);
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
  if (zipStream.avail_out == 0) {
    // Consume output:
    
  }

  int rc = deflate(&zipStream, Z_NO_FLUSH);

  switch(rc) {
  case Z_OK:
    break;

    // TODO: Throw Exception
  }
}


void GzipOutputStream::finish()
{
  int rc = deflate(&zipStream, Z_FINISH);

  switch(rc) {
  case Z_OK:
    break;
    // TODO: Throw Exception
  }
}


void GzipOutputStream::close()
{
  finish();
  out->close();
}

