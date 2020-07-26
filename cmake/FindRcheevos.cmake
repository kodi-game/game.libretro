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


find_path(RCHEEVOS_INCLUDE_DIR rcheevos.h
                              PATH_SUFFIXES rcheevos
                              PATHS ${PC_RCHEEVOS_INCLUDEDIR})

find_library(RCHEEVOS_LIBRARY NAMES rcheevoslib
                             PATH_SUFFIXES rcheevos
                             PATHS ${PC_RCHEEVOS_LIBDIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Rcheevos REQUIRED_VARS RCHEEVOS_LIBRARY RCHEEVOS_INCLUDE_DIR)

if(RCHEEVOS_FOUND)
  set(RCHEEVOS_INCLUDE_DIRS ${RCHEEVOS_INCLUDE_DIR})
  set(RCHEEVOS_LIBRARIES ${RCHEEVOS_LIBRARY})
endif()

mark_as_advanced(RCHEEVOS_LIBRARY RCHEEVOS_INCLUDE_DIR)
