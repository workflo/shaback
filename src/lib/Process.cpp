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

#include <iostream>
#ifdef HAVE_SCHED_H
# include <sched.h>
#endif

#ifdef HAVE_ERRNO_H
# include <errno.h>
#endif

#if defined(HAVE_STDARG_H) || defined(JAKELIB_WIN32API)
# include <stdarg.h>
#endif

#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

//#ifdef HAVE_SIGNAL_H
//# include <signal.h>
//#endif

#include <fcntl.h>
#include <signal.h>

#include "Exception.h"
#include "InputStream.h"
#include "OutputStream.h"
#include "Process.h"

using namespace std;

class PipeInputStream: public InputStream
{
  public:

#if defined(JAKELIB_WIN32API)

    PipeInputStream(HANDLE pipe)
    {
      this->pipe = pipe;
    }

#else

    PipeInputStream(int pipe)
    {
      this->pipe = pipe;
    }

#endif

    ~PipeInputStream()
    {
      close();
    }

    int read()
    {
      char b;
      long r;

      r = read(&b, 1);
      if (r == 1) {
        return b & 0xff;
      } else if (r == 0) {
        return -2;
      } else {
        return -1;
      }
    }

    int read(char* b, int len)
    {
      if (len == 0)
        return 0;

#if defined(WIN32API)

      DWORD r;

      if (!ReadFile(pipe, &b[offset], len, &r, NULL)) {
        if (GetLastError() == ERROR_BROKEN_PIPE) {
          return -1;
        }
        else {
          throw new IOException(System::explainErrorCode(GetLastError())
          ->plus( JAKELIB_AT2("jakelib.io.PipeInputStream.read")));
        }
      }

      return r;

#else

      int r;

      if ((r = ::read(pipe, (char*) b, len)) < 0) {
        if (errno == EAGAIN || errno == ENOENT || errno == EINTR) {
          return 0;
        } else {
          throw Exception::errnoToException();
        }
      }

      if (r == 0)
        return -1;
      else
        return r;

#endif
    }

    void close()
    {
#if defined(WIN32API)
      CloseHandle(pipe);
#else
      ::close(pipe);
#endif
    }

    void reset()
    {
    }

    void setBlocking(bool on)
    {
#if !defined(WIN32API)
      if (fcntl(pipe, F_SETFL, O_NONBLOCK) == -1) {
        throw Exception::errnoToException();
      }

#else
      throw new UnsupportedOperationException(JAKELIB_ONDEMAND(jakelib2_strings[0], new jakelib::lang::String(chars_jakelib2_str_0, 0, 47))
          ->plus( JAKELIB_AT2("jakelib.io.PipeInputStream.setBlocking")));
#endif
    }

  protected:

#if defined(WIN32API)
    HANDLE pipe;
#else
    int pipe;
#endif
};



class PipeOutputStream : public OutputStream
{
  public:
#if defined(WIN32API)

PipeOutputStream(HANDLE pipe)
{
  this->pipe = pipe;
}

#else

PipeOutputStream(int pipe)
{
  this->pipe = pipe;
}

#endif

~PipeOutputStream()
{
  close();
}


void write(int b)
{
  char c = (char) b;

#if !defined(WIN32API)
  ::write(pipe, &c, 1);
#else
  throw new UnsupportedOperationException(new String("PipeOutputStream::write is not yet supported for Borland C++"));
#endif
}


void write(char* b, int len)
{
  if (len == 0)
    return;

#if defined(WIN32API)

  DWORD w, totalWritten = 0;

  do {
    if (!WriteFile(pipe, &b[offset + totalWritten], len - totalWritten, &w, NULL)) {
      throw new IOException(System::explainErrorCode(GetLastError())
                        ->plus( JAKELIB_AT2("jakelib.io.PipeOutputStream.write")));
    }
    totalWritten += w;
  } while (totalWritten < (DWORD)len);

#else

  int totalWritten = 0;

  do {
    int w = ::write(pipe, &b[totalWritten], len - totalWritten);

    if (w < 0) {
      throw Exception::errnoToException();
    }
    totalWritten += w;
  } while (totalWritten < len);

#endif
}


void close()
{
#if defined(JAKELIB_WIN32API)
  CloseHandle(pipe);
#else
  ::close(pipe);
#endif
}

  protected:

#if defined(JAKELIB_WIN32API)
    HANDLE pipe;
#else
    int pipe;
#endif
};


Process::Process(const char *file, char *const argv[])
{
  init1();

#if defined(WIN32API)

  ZeroMemory(&procInfo, sizeof(PROCESS_INFORMATION));

  SECURITY_ATTRIBUTES secAttr;
  secAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  secAttr.bInheritHandle = TRUE;
  secAttr.lpSecurityDescriptor = NULL;

  // Create pipes for stdin, stdout and stderr:
  HANDLE childInPipe, childOutPipe, childErrPipe;

  if (!CreatePipe(&inPipe, &childOutPipe, &secAttr, 0)) {
    throw new IOException(`"CreatePipe failed for stdin: "`
        .. System::explainErrorCode(GetLastError())
    .. JAKELIB_AT2("jakelib.lang.Process.Process"));
  }

  if (!CreatePipe(&errPipe, &childErrPipe, &secAttr, 0)) {
    throw new IOException(`"CreatePipe failed for stderr: "`
        .. System::explainErrorCode(GetLastError())
    .. JAKELIB_AT2("jakelib.lang.Process.Process"));
  }

  if (!CreatePipe(&childInPipe, &outPipe, &secAttr, 0)) {
    throw new IOException(`"CreatePipe failed for stdout: "`
        .. System::explainErrorCode(GetLastError())
    .. JAKELIB_AT2("jakelib.lang.Process.Process"));
  }

  // Duplicate write end of StdOut pipe :
  HANDLE dupHandle;
  if (!DuplicateHandle(GetCurrentProcess(), outPipe,
      GetCurrentProcess(), &dupHandle, 0,
      FALSE, DUPLICATE_SAME_ACCESS)) {
    throw new IOException(`"DuplicateHandle failed: "`
        .. System::explainErrorCode(GetLastError())
    .. JAKELIB_AT2("jakelib.lang.Process.Process"));
  }
  CloseHandle(outPipe);
  outPipe = dupHandle;

  // Build up one command line string:
  va_list ptr;
  va_start(ptr, cmdline);

  StringBuffer* args = new StringBuffer();
  String* s;
  args->append(cmdline)->append(' ');
  while ((s = va_arg(ptr, String*)) != null) {
    args->append(s)->append(' ');
  }

  // Build up Startup-Info structure:
  STARTUPINFO startInfo;
  ZeroMemory(&startInfo, sizeof(STARTUPINFO));
  startInfo.cb = sizeof(STARTUPINFO);
  startInfo.wShowWindow = SW_SHOW;
  startInfo.dwFlags = STARTF_USESTDHANDLES;
  startInfo.hStdInput = childInPipe;
  startInfo.hStdOutput = childOutPipe;
  startInfo.hStdError = childErrPipe;

  // Actually create the new process:
  if (!CreateProcess(NULL, args->toString()->latin1(),
      NULL, NULL, TRUE,
      0, 0, 0,
      &startInfo,
      &procInfo)) {
    throw new IOException(System::explainErrorCode(GetLastError()) .. `":"`
        .. System::eol .. args
        .. JAKELIB_AT2("jakelib.lang.Process.Process"));
  }

  // Close children's end of all pipes:
  CloseHandle(childInPipe);
  CloseHandle(childOutPipe);
  CloseHandle(childErrPipe);

#else

  int stdoutPipes[2], stdinPipes[2], stderrPipes[2];
  pid_t pid;

  if (pipe(stdoutPipes) != 0) {
    throw IOException(string("Cannot create stdout pipe for process: ").append(file));
  }
  if (pipe(stdinPipes) != 0) {
    throw IOException(string("Cannot create stdin pipe for process: ").append(file));
  }
  if (pipe(stderrPipes) != 0) {
    throw IOException(string("Cannot create stderr pipe for process: ").append(file));
  }

  childPid = fork();
  if (childPid < 0) {
    throw IOException("Unable to fork new process.");
  }

  if (childPid == 0) {
    // Child process:

    dup2(stdoutPipes[1], fileno(stdout));
    dup2(stderrPipes[1], fileno(stderr));
    dup2(stdinPipes[0], fileno(stdin));

    execvp(file, argv);
    int e = errno;
    cerr << "Cannot exec: " << file << " - " << strerror(e);
    exit(2);
  } else {
    // Parent process:

    inPipe = stdoutPipes[0];
    outPipe = stdinPipes[1];
    errPipe = stderrPipes[0];

    close(stdinPipes[0]);
    close(stdoutPipes[1]);
    close(stderrPipes[1]);

    // FIXME: Throw Exception if execvp failed!
  }

#endif
}

Process::~Process()
{
#if defined(WIN32API)

  if (procInfo.hProcess != INVALID_HANDLE_VALUE) {
    CloseHandle(procInfo.hProcess);
    CloseHandle(procInfo.hThread);
  }

#endif

  if (inputStream) delete inputStream;
  if (outputStream) delete outputStream;
  if (errorStream) delete errorStream;
}

void Process::init1()
{
  inputStream = 0;
  outputStream = 0;
  errorStream = 0;
  terminated = false;
  _exitValue = 255;
}

InputStream* Process::getInputStream()
{
  if (inputStream == 0) {
    inputStream = new PipeInputStream(inPipe);
    inputStream;
  }
  return inputStream;
}

InputStream* Process::getErrorStream()
{
  if (errorStream == 0) {
    errorStream = new PipeInputStream(errPipe);
    errorStream;
  }
  return errorStream;
}

OutputStream* Process::getOutputStream()
{
  if (outputStream == 0) {
    outputStream = new PipeOutputStream(outPipe);
    outputStream;
  }
  return outputStream;
}

void Process::destroy()
{
#if defined(JAKELIB_WIN32API)

  TerminateProcess(procInfo.hProcess, -1);
  CloseHandle(procInfo.hProcess);
  CloseHandle(procInfo.hThread);
  procInfo.hProcess = INVALID_HANDLE_VALUE;
  procInfo.hThread = INVALID_HANDLE_VALUE;

#else

  kill(childPid, SIGKILL);

#endif
}

void Process::waitFor()
{
#if defined(WIN32API)

  WaitForSingleObject(procInfo.hProcess, INFINITE);

#else

  // FIXME: error handling
  waitpid(childPid, &_exitValue, WUNTRACED);
  terminated = true;

#endif
}

int Process::exitValue()
{
  if (terminated) {
    return _exitValue;
  }

#if defined(WIN32API)

  if (!GetExitCodeProcess(procInfo.hProcess, &_exitValue)) {
    throw new IOException(`"Cannot determine exit code: #"` .. (jlong) GetLastError()
        .. JAKELIB_AT2("jakelib.lang.Process.exitValue"));
  }
  if (_exitValue == STILL_ACTIVE) {
    throw new IllegalThreadStateException(`"Subprocess has not yet terminated"`
        .. JAKELIB_AT2("jakelib.lang.Process.exitValue"));
  }

#else

  if (waitpid(childPid, &_exitValue, WNOHANG) <= 0) {
    throw IOException("Subprocess has not yet terminated");
    if (WIFEXITED(_exitValue)) {
      _exitValue = WEXITSTATUS(_exitValue);
    } else if (WIFSTOPPED(_exitValue)) {
      throw IOException(string("Process was stopped"));//.append(_exitValue));
    } else {
      throw IOException(string("Process aborted abnormally"));//.append(_exitValue));
    }
    _exitValue = _exitValue & 0xff;
  }

#endif

  terminated = true;
  return _exitValue;
}
