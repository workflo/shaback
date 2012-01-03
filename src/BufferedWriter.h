#ifndef SHABACK_BufferedOutputStream_H
#define SHABACK_BufferedOutputStream_H

#include <string>
#include "OutputStream.h"

/**
 * \c BufferedWriter provides a \c Writer that uses an internal
 * buffer to improve performance when subsequently writing single
 * characters or short character sequences.
 *
 * @class BufferedWriter
 * @author Florian 'Overflo' Wolff (florian@donuz.de)
 */
class BufferedWriter: public OutputStream {
public:

	/**
	 * @brief Creates a new \c BufferedWriter from a given \c Writer object.
	 * Uses the given \c bufferSize to allocate the output buffer.
	 *
	 * @param writer The \c Writer to write to
	 * @param bufferSize The buffer size in bytes, default is 8kB.
	 */
	BufferedWriter(OutputStream* out, int bufferSize = 8192);

	~BufferedWriter();

	void write(const char* buf, int len);

	void write(int c);

	void write(std::string& str);

	void write(const char* str);

	void close();

	void flush();

	/**
	 * Writes the sustem-dependend newline character sequence.
	 */
	void newLine();

private:
	/** The current buffer size */
	int bufferSize;

	/** The underlaying \c OutputStream */
	OutputStream* out;

	/** The output buffer */
	char* buffer;

	/** The number of characters already in the output buffer */
	int nChars;

	/** Asserts the the \c Writer is not yet closed. */
	void ensureOpen();
};
#endif // SHABACK_BufferedOutputStream_H
