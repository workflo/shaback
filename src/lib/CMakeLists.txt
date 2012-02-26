
find_package(ZLIB REQUIRED)
find_package(BZip2 REQUIRED)
find_package(Lzma REQUIRED)

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

include_directories(${LZMA_INCLUDE_DIR})
add_library(shabacklib STATIC ${shaback_lib_SRCS})
