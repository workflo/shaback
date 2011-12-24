#ifndef SHABACK_OutputStream_H
#define SHABACK_OutputStream_H

/**
 * This abstract class forms the base of the hierarchy of classes that 
 * write output as a stream of bytes.
 *
 * It provides a common set of methods
 * for writing bytes to stream. Subclasses implement and/or extend these
 * methods to write bytes in a particular manner or to a particular 
 * destination such as a file on disk or network connection.
 *
 * @class OutputStream
 * @author Florian 'Overflo' Wolff (florian@donuz.de)
 */
class OutputStream
{
public:

  /**
   * Writes the specified byte value to this output stream.
   *
   * @param b The byte value to be written. Only the lower 8 bits
   *          are written, all remaining bits are ignored.
   */
  virtual void write(int b) = 0;

  /**
   * Writes a number of bytes to the output stream.
   *
   * @param b The byte array to be written.
   * @param offset Offset in byte array \a b.
   * @param len Number of bytes to be written.
   */
  virtual void write(const char* b, int offset, int len);

  /**
   * Closes the output stream.
   */
  virtual void close() = 0;
};
#endif // SHABACK_OutputStream_H
