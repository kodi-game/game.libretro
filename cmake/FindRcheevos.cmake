# FindRcheevos
# -----------
# Finds the Rcheevos library
#
# This will define the following variables::
#
# RCHEEVOS_FOUND - system has Rcheevos
# RCHEEVOS_INCLUDE_DIRS - the Rcheevos include directory
# RCHEEVOS_LIBRARIES - the Rcheevos libraries
#

if(ENABLE_INTERNAL_RCHEEVOS)
  include(ExternalProject)
  file(STRINGS ${CMAKE_SOURCE_DIR}/depends/common/rcheevos/rcheevos.txt rcheevosurl REGEX "^rcheevos[\t ]*.+$")
  string(REGEX REPLACE "^rcheevos[\t ]*(.+)[\t ]*$" "\\1" url "${rcheevosurl}")

  # allow user to override the download URL with a local tarball
  # needed for offline build envs
  if(RCHEEVOS_URL)
      get_filename_component(RCHEEVOS_URL "${RCHEEVOS_URL}" ABSOLUTE)
  else()
      set(RCHEEVOS_URL ${url})
  endif()
  if(VERBOSE)
      message(STATUS "RCHEEVOS_URL: ${RCHEEVOS_URL}")
  endif()

  set(RCHEEVOS_INCLUDE_DIR ${CMAKE_BINARY_DIR}/build/depends/include/rcheevos)
  set(RCHEEVOS_LIBRARY ${CMAKE_BINARY_DIR}/build/depends/lib/librcheevoslib.a)

  externalproject_add(rcheevos
                      URL ${RCHEEVOS_URL}
                      DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/download
                      PREFIX ${CMAKE_BINARY_DIR}/build/rcheevos
                      PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/depends/common/rcheevos/CMakeLists.txt ${CMAKE_BINARY_DIR}/build/rcheevos/src/rcheevos/
                      CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/build/depends
                      BUILD_BYPRODUCTS ${RCHEEVOS_LIBRARY})
else()
  find_path(RCHEEVOS_INCLUDE_DIR rcheevos.h
                                PATH_SUFFIXES rcheevos
                                PATHS ${PC_RCHEEVOS_INCLUDEDIR})

  find_library(RCHEEVOS_LIBRARY NAMES rcheevoslib
                               PATH_SUFFIXES rcheevos
                               PATHS ${PC_RCHEEVOS_LIBDIR})
endif()
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Rcheevos REQUIRED_VARS RCHEEVOS_LIBRARY RCHEEVOS_INCLUDE_DIR)

if(RCHEEVOS_FOUND)
  set(RCHEEVOS_INCLUDE_DIRS ${RCHEEVOS_INCLUDE_DIR})
  set(RCHEEVOS_LIBRARIES ${RCHEEVOS_LIBRARY})
endif()

mark_as_advanced(RCHEEVOS_LIBRARY RCHEEVOS_INCLUDE_DIR)
