# - Find zstd
# Find the native ZSTD includes and library
#
#  ZSTD_INCLUDE_DIR - where to find zstd.h, etc.
#  ZSTD_LIBRARIES   - List of libraries when using zstd.
#  ZSTD_FOUND       - True if zstd found.


IF (ZSTD_INCLUDE_DIR)
  # Already in cache, be silent
  SET(ZSTD_FIND_QUIETLY TRUE)
ENDIF (ZSTD_INCLUDE_DIR)

FIND_PATH(ZSTD_INCLUDE_DIR zstd.h
  /usr/local/include
  /usr/include
  /opt/local/include
)

SET(ZSTD_NAMES zstd)
FIND_LIBRARY(ZSTD_LIBRARY
  NAMES ${ZSTD_NAMES}
  PATHS /usr/lib /usr/local/lib /opt/local/lib
)

IF (ZSTD_INCLUDE_DIR AND ZSTD_LIBRARY)
   SET(ZSTD_FOUND TRUE)
    SET( ZSTD_LIBRARIES ${ZSTD_LIBRARY} )
ELSE (ZSTD_INCLUDE_DIR AND ZSTD_LIBRARY)
   SET(ZSTD_FOUND FALSE)
   SET( ZSTD_LIBRARIES )
ENDIF (ZSTD_INCLUDE_DIR AND ZSTD_LIBRARY)

IF (ZSTD_FOUND)
   IF (NOT ZSTD_FIND_QUIETLY)
      MESSAGE(STATUS "Found ZSTD: ${ZSTD_LIBRARY}")
   ENDIF (NOT ZSTD_FIND_QUIETLY)
ELSE (ZSTD_FOUND)
   IF (ZSTD_FIND_REQUIRED)
      MESSAGE(STATUS "Looked for zstd libraries named ${ZSTDS_NAMES}.")
      MESSAGE(FATAL_ERROR "Could NOT find zstd library")
   ENDIF (ZSTD_FIND_REQUIRED)
ENDIF (ZSTD_FOUND)

MARK_AS_ADVANCED(
  ZSTD_LIBRARY
  ZSTD_INCLUDE_DIR
)
