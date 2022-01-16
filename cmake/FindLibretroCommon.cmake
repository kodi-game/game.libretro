# FindLibretroCommon.cmake
# -------
# Finds the libretro-common headers
#
# This will define the following variables:
#
# LIBRETRO_COMMON_FOUND - libretro-common was found
# LIBRETRO_COMMON_INCLUDE_DIRS - the libretro include directory containing libretro.h
#

find_path(LIBRETRO_COMMON_INCLUDE_DIR NAMES libretro.h
                                      PATH_SUFFIXES libretro-common)

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
