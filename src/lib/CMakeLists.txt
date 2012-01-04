set(shaback_lib_SRCS File.cpp FileOutputStream.cpp FileInputStream.cpp Sha1.cpp Sha256.cpp Exception.cpp OutputStream.cpp InputStream.cpp DeflateOutputStream.cpp DeflateInputStream.cpp StandardOutputStream.cpp StandardInputStream.cpp BufferedWriter.cpp BufferedReader.cpp BlowfishOutputStream.cpp BlowfishInputStream.cpp Date.cpp)

add_library(shabacklib STATIC ${shaback_lib_SRCS})
