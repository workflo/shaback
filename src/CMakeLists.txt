project(cshaback)
set (SHABACK_VERSION_MAJOR 1)
set (SHABACK_VERSION_MINOR 12)
set (CMAKE_MODULE_PATH CMakeModules)
set (CMAKE_VERBOSE_MAKEFILE true)

cmake_minimum_required(VERSION 2.8)
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Lua51 REQUIRED)
find_package(GDBM REQUIRED)

# Erst ab cmake 2.8.5:
if (CMAKE_VERSION VERSION_GREATER "2.8.4")
  include (GNUInstallDirs)
else (CMAKE_VERSION VERSION_GREATER "2.8.4")
  set (CMAKE_INSTALL_SYSCONFDIR "/etc")
endif (CMAKE_VERSION VERSION_GREATER "2.8.4")


#ADD_SUBDIRECTORY(include)
#ADD_SUBDIRECTORY(src)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${LUA_INCLUDE_DIR})

set(cshaback_SRCS cshaback.cpp main.cpp RuntimeConfig.cpp Sha1.cpp File.cpp Repository.cpp BackupRun.cpp)
add_executable(cshaback ${cshaback_SRCS})
target_link_libraries(cshaback ${ZLIB_LIBRARY} ${OPENSSL_CRYPTO_LIBRARIES} ${LUA_LIBRARIES} ${GDBM_LIBRARY})

install(DIRECTORY etc/shaback DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})
install(PROGRAMS cshaback DESTINATION bin)

#INCLUDE(CPack)

# lzo
# acl
# xattr



# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/ShabackConfig.h.in"
  "${PROJECT_SOURCE_DIR}/ShabackConfig.h"
  )
