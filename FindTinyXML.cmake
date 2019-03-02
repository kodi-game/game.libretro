# FindTinyXML
# -----------
# Finds the TinyXML library
#
# This will define the following variables::
#
# TINYXML_FOUND - system has TinyXML
# TINYXML_INCLUDE_DIRS - the TinyXML include directory
# TINYXML_LIBRARIES - the TinyXML libraries
#

if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_TINYXML tinyxml QUIET)
endif()

find_path(TINYXML_INCLUDE_DIR tinyxml.h
                              PATH_SUFFIXES tinyxml
                              PATHS ${PC_TINYXML_INCLUDEDIR})

find_library(TINYXML_LIBRARY NAMES tinyxml tinyxmlSTL
                             PATH_SUFFIXES tinyxml
                             PATHS ${PC_TINYXML_LIBDIR})
set(TINYXML_VERSION ${PC_TINYXML_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TinyXML
                                  REQUIRED_VARS TINYXML_LIBRARY TINYXML_INCLUDE_DIR
                                  VERSION_VAR TINYXML_VERSION)

if(TINYXML_FOUND)
  set(TINYXML_INCLUDE_DIRS ${TINYXML_INCLUDE_DIR})
  set(TINYXML_LIBRARIES ${TINYXML_LIBRARY})
endif()

mark_as_advanced(TINYXML_INCLUDE_DIR TINYXML_LIBRARY)
