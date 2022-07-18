# FindLibretroCommon.cmake
# -------
# Finds the libretro-common headers
#
# This will define the following variables:
#
# LIBRETRO_COMMON_FOUND - libretro-common was found
# LIBRETRO_COMMON_INCLUDE_DIRS - the libretro include directory containing libretro.h
#

if(ENABLE_INTERNAL_LIBRETROCOMMON)
  include(ExternalProject)
  file(STRINGS ${CMAKE_SOURCE_DIR}/depends/common/libretro-common/libretro-common.txt libretrocommonurl REGEX "^libretro-common[\t ]*.+$")
  string(REGEX REPLACE "^libretro-common[\t ]*(.+)[\t ]*$" "\\1" url "${libretrocommonurl}")

  # allow user to override the download URL with a local tarball
  # needed for offline build envs
  if(LIBRETROCOMMON_URL)
      get_filename_component(LIBRETROCOMMON_URL "${LIBRETROCOMMON_URL}" ABSOLUTE)
  else()
      set(LIBRETROCOMMON_URL ${url})
  endif()
  if(VERBOSE)
      message(STATUS "LIBRETROCOMMON_URL: ${LIBRETROCOMMON_URL}")
  endif()

  set(LIBRETRO_COMMON_INCLUDE_DIR ${CMAKE_BINARY_DIR}/build/depends/include/libretro-common)

  externalproject_add(libretro-common
                      URL ${LIBRETROCOMMON_URL}
                      DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/download
                      PREFIX ${CMAKE_BINARY_DIR}/build/libretro-common
                      CONFIGURE_COMMAND ""
                      BUILD_COMMAND ""
                      INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/libretro-common/src/libretro-common/include ${LIBRETRO_COMMON_INCLUDE_DIR}
                      BUILD_BYPRODUCTS ${LIBRETRO_COMMON_INCLUDE_DIR}/libretro.h
                      BUILD_IN_SOURCE 1)
else()
  find_path(LIBRETRO_COMMON_INCLUDE_DIR NAMES libretro.h
                                        PATH_SUFFIXES libretro-common)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  LibretroCommon
    REQUIRED_VARS LIBRETRO_COMMON_INCLUDE_DIR
)

if(LibretroCommon_FOUND)
  set(LIBRETRO_COMMON_FOUND true)
  set(LIBRETRO_COMMON_INCLUDE_DIRS ${LIBRETRO_COMMON_INCLUDE_DIR})
endif()

mark_as_advanced(LIBRETRO_COMMON_INCLUDE_DIR)
