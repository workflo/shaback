set(shaback_lib_SRCS
		     BlowfishOutputStream.cpp
		     BlowfishInputStream.cpp
		     BufferedWriter.cpp
		     BufferedReader.cpp
		     Date.cpp
		     DeflateOutputStream.cpp
		     DeflateInputStream.cpp
		     Exception.cpp
		     File.cpp
		     FileOutputStream.cpp
		     FileInputStream.cpp
		     InputStream.cpp
		     OutputStream.cpp
		     Properties.cpp
		     Sha1.cpp
		     Sha256.cpp
		     StandardOutputStream.cpp
		     StandardInputStream.cpp
		     )

add_library(shabacklib STATIC ${shaback_lib_SRCS})
