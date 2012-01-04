#ifndef SHABACK_BufferedReader_H
#define SHABACK_BufferedReader_H

#include <string>
#include "InputStream.h"

/**
 * \c BufferedReader provides a \c Reader that uses an internal
 * buffer to improve performance when subsequently reading single
 * characters.
 *
 * It also provides the \c readString() method that reads a full line
 * of text (until the next newline character).
 *
 * @class BufferedReader
 * @version $Id: BufferedReader.h,v 1.24 2003-09-13 10:29:03 florian Exp $
 * @author Florian 'Overflo' Wolff (florian@donuz.de)
 */
class BufferedReader: public InputStream
{
  public:

    /**
     * @brief Creates a new \c BufferedReader from a given \c Reader object.
     * Uses the given \c bufferSize to allocate the input buffer.
     *
     * @param reader The \c Reader to be buffered.
     * @param bufferSize Buffer size in bytes, default is 8kB.
     */
    BufferedReader(InputStream* in, int bufferSize = 8192);

    /**
     * @brief The destructor.
     */
    ~BufferedReader();

    void close();

    virtual int read();

    virtual int read(char* buf, int len);

  private:

    /** @brief The current buffer size */
    int bufferSize;

    InputStream* in;
    char* buffer;

    int nChars;
    int nextChar;

    /**
     * @throws IOException If the reader is closed
     */
    void ensureOpen();

    /**
     * @brief (Re)Fills the input buffer.
     */
    void fill();
};
#endif // SHABACK_BufferedReader_H
