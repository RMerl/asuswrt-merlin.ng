# Finds the Berkeley DB Library
#
#  BDB_FOUND          - True if Berkeley DB found.
#  BDB_INCLUDE_DIR    - Directory to include to get Berkeley DB headers
#  BDB_LIBRARY        - Library to link against for the Berkeley DB
#

if(BDB_INCLUDE_DIR AND BDB_LIBRARY)
  # Already in cache, be silent
  set(BDB_FIND_QUIETLY TRUE)
endif()

# Look for the header file.
find_path(
  BDB_INCLUDE_DIR
  NAMES db.h
  HINTS /usr/local/opt/db/include
  DOC "Include directory for the Berkeley DB library"
)
mark_as_advanced(BDB_INCLUDE_DIR)

# Look for the library.
find_library(
  BDB_LIBRARY
  NAMES db
  HINTS /usr/local/opt/db4/lib
  DOC "Libraries to link against for the Berkeley DB"
)
mark_as_advanced(BDB_LIBRARY)

# Copy the results to the output variables.
if(BDB_INCLUDE_DIR AND BDB_LIBRARY)
  set(BDB_FOUND 1)
else()
  set(BDB_FOUND 0)
endif()

if(BDB_FOUND)
  if(NOT BDB_FIND_QUIETLY)
    message(STATUS "Found Berkeley DB header files in ${BDB_INCLUDE_DIR}")
    message(STATUS "Found Berkeley libraries: ${BDB_LIBRARY}")
  endif()
else()
  if(BDB_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find Berkeley DB")
  else()
    message(STATUS "Optional package Berkeley DB was not found")
  endif()
endif()
