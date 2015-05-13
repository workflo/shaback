# - Find dialog
# Find the native DIALOG includes and library
#
#  DIALOG_INCLUDE_DIR - where to find dialog.h, etc.
#  DIALOG_LIBRARIES   - List of libraries when using dialog.
#  DIALOG_FOUND       - True if dialog found.


IF (DIALOG_INCLUDE_DIR)
  # Already in cache, be silent
  SET(DIALOG_FIND_QUIETLY TRUE)
ENDIF (DIALOG_INCLUDE_DIR)

FIND_PATH(DIALOG_INCLUDE_DIR dialog.h
  /usr/local/include
  /usr/include
  /opt/local/include
)

SET(DIALOG_NAMES dialog)
FIND_LIBRARY(DIALOG_LIBRARY
  NAMES ${DIALOG_NAMES}
  PATHS /usr/lib /usr/local/lib /opt/local/lib
)

IF (DIALOG_INCLUDE_DIR AND DIALOG_LIBRARY)
   SET(DIALOG_FOUND TRUE)
    SET( DIALOG_LIBRARIES ${DIALOG_LIBRARY} )
ELSE (DIALOG_INCLUDE_DIR AND DIALOG_LIBRARY)
   SET(DIALOG_FOUND FALSE)
   SET( DIALOG_LIBRARIES )
ENDIF (DIALOG_INCLUDE_DIR AND DIALOG_LIBRARY)

IF (DIALOG_FOUND)
   IF (NOT DIALOG_FIND_QUIETLY)
      MESSAGE(STATUS "Found DIALOG: ${DIALOG_LIBRARY}")
   ENDIF (NOT DIALOG_FIND_QUIETLY)
ELSE (DIALOG_FOUND)
   IF (DIALOG_FIND_REQUIRED)
      MESSAGE(STATUS "Looked for dialog libraries named ${DIALOGS_NAMES}.")
      MESSAGE(FATAL_ERROR "Could NOT find dialog library")
   ENDIF (DIALOG_FIND_REQUIRED)
ENDIF (DIALOG_FOUND)

MARK_AS_ADVANCED(
  DIALOG_LIBRARY
  DIALOG_INCLUDE_DIR
)
