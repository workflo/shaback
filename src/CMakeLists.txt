project(shaback)
set (SHABACK_VERSION_MAJOR 1)
set (SHABACK_VERSION_MINOR 23)
set (CMAKE_MODULE_PATH CMakeModules)
set (CMAKE_VERBOSE_MAKEFILE true)

cmake_minimum_required(VERSION 2.8)
find_package(ZLIB QUIET)
find_package(BZip2 REQUIRED)
find_package(Lzma QUIET)
find_package(ZStd QUIET)
find_package(OpenSSL QUIET)
find_package(Lua51 REQUIRED)
find_package(GDBM REQUIRED)
find_package(NCurses QUIET)

include(CheckTypeSize)
check_type_size("long" SIZEOF_LONG)
check_type_size("long long" SIZEOF_LONG_LONG)

ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(dialog)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${LUA_INCLUDE_DIR})

add_executable(shaback
        BackupRun.cpp
        BackupsetSelector.cpp
        Cache.cpp 
        GarbageCollection.cpp
        History.cpp
        main.cpp 
        Repository.cpp
        RestoreReport.cpp
        RestoreRun.cpp
        RuntimeConfig.cpp 
        shaback.cpp 
        ShabackException.cpp
        ShabackInputStream.cpp
        ShabackOutputStream.cpp
        SplitFileIndexReader.cpp
        TreeFileEntry.cpp
    )

target_link_libraries(shaback 
  shabacklib
  shabackdialog
  ${ZLIB_LIBRARY}
  ${OPENSSL_CRYPTO_LIBRARIES}
  ${LUA_LIBRARIES}
  ${GDBM_LIBRARY}
  ${BZIP2_LIBRARIES}
  ${ZSTD_LIBRARIES}
  ${LZMA_LIBRARIES}
  ${NCURSES_LIBRARIES}
)

#add_executable(visichunk
#       VisiChunk.cpp
#       )

#target_link_libraries(visichunk
#  shabacklib)

install(DIRECTORY etc/shaback DESTINATION "${CMAKE_INSTALL_PREFIX}/etc")
install(PROGRAMS shaback DESTINATION bin)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/ShabackConfig.h.in"
  "${PROJECT_SOURCE_DIR}/ShabackConfig.h"
  )
