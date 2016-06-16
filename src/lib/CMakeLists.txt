cmake_minimum_required(VERSION 2.8)

set (CMAKE_MODULE_PATH CMakeModules)
INCLUDE (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
find_package(ZLIB REQUIRED)
find_package(BZip2 REQUIRED)
find_package(OpenSSL QUIET)
find_package(Lzma QUIET)

CHECK_FUNCTION_EXISTS (fstat64           HAVE_FSTAT64)
CHECK_FUNCTION_EXISTS (stat64            HAVE_STAT64)
CHECK_FUNCTION_EXISTS (lstat64           HAVE_LSTAT64)

IF (OPENSSL_INCLUDE_DIR)
    include_directories(AFTER ${OPENSSL_INCLUDE_DIR})
ENDIF (OPENSSL_INCLUDE_DIR)

set(shaback_lib_SRCS
		     BlowfishOutputStream.cpp
		     BlowfishInputStream.cpp
		     BufferedWriter.cpp
		     BufferedReader.cpp
                     BzInputStream.cpp
                     BzOutputStream.cpp
		     Date.cpp
		     DeflateOutputStream.cpp
		     DeflateInputStream.cpp
		     Digest.cpp
		     Exception.cpp
		     File.cpp
		     FileOutputStream.cpp
		     FileInputStream.cpp
		     InputStream.cpp
                     LzmaInputStream.cpp
                     LzmaOutputStream.cpp
		     OutputStream.cpp
		     Properties.cpp
		     Sha1.cpp
		     Sha256.cpp
		     StandardOutputStream.cpp
		     StandardInputStream.cpp
		     )

IF (LZMA_FOUND)
    include_directories(${LZMA_INCLUDE_DIR})
ENDIF (LZMA_FOUND)

add_library(shabacklib STATIC ${shaback_lib_SRCS})

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "config.h.in"
  "config.h"
  )
