/*
 * shaback - A hash digest based backup tool.
 * Copyright (C) 2012 Florian Wolff (florian@donuz.de)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SHABACK_Process_H
#define SHABACK_Process_H

#include <string>
#include <vector>
#include <sys/stat.h>
#include <stdio.h>

#ifdef WIN32
# include <windows.h>
# include <direct.h>
#endif

class OutputStream;
class InputStream;

/**
 * An instance of \c Process can be used to start a native process,
 * watch its output and pass input.
 *
 * \b Example:
 * \code
 *   Process process = new Process("ls", "-l", "-a", null);
 *   InputStream* input = process->getInputStream();
 *   process->waitFor();
 *   int rc = process.exitValue();
 * \endcode
 *
 * @class Process
 * @author Florian 'Overflo' Wolff (florian@donuz.de)
 */
class Process
{
  public:
    /**
     * The given program is executed, the argument list must end in a \c null
     * element.
     *
     * @param progname Filename of the program to be executed
     * @param ... <tt>null</tt>-terminated argument list
     */
    Process(const char *file, char *const argv[]);

    ~Process();

    /**
     * @brief Gets the output stream of the subprocess' <tt>stdin</tt> stream.
     */
    OutputStream * getOutputStream();

    /**
     * @brief Gets the input stream for the subprocess' <tt>stdout</tt> stream.
     */
    InputStream * getInputStream();

    /**
     * @brief Gets the input stream for the subprocess' <tt>stderr</tt> stream.
     */
    InputStream * getErrorStream();

    /**
     * @brief Kills the subprocess.
     */
    void destroy();

    /**
     * @brief Waits - if necessary - for the subprocess to terminate.
     */
    void waitFor();

    /**
     * @brief Returns the exit value of the subprocess.
     *
     * @throws IllegalThreadStateException If the process has not yet terminated.
     */
    int exitValue();

  protected:

    /** Holds the process' stdout stream */
    OutputStream * outputStream;

    /** Holds the process' stdin stream */
    InputStream * inputStream;

    /** Holds the process' stderr stream */
    InputStream * errorStream;

  private:

#if defined(WIN32)
    PROCESS_INFORMATION procInfo;
    DWORD _exitValue;

    /** StdOut pipe (stdin for the child process) */
    HANDLE outPipe;

    /** StdErr pipe (stderr for the child process) */
    HANDLE errPipe;

    /** StdIn pipe (stdout for the child process) */
    HANDLE inPipe;
#else
    pid_t childPid;
    int _exitValue;

    /** StdOut pipe (stdin for the child process) */
    int outPipe;

    /** StdErr pipe (stderr for the child process) */
    int errPipe;

    /** StdIn pipe (stdout for the child process) */
    int inPipe;
#endif
    bool terminated;

    void init1();

    void init2();

};

#endif // SHABACK_Process_H
