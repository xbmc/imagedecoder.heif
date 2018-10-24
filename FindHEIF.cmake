#.rst:
# FindHEIF
# --------
# Finds the HEIF library
#
# This will will define the following variables::
#
# HEIF_FOUND - system has heif
# HEIF_INCLUDE_DIRS - the heif include directory
# HEIF_LIBRARIES - the heif libraries
#
# and the following imported targets::
#
#   HEIF::HEIF - The HEIF library

if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_HEIF libheif QUIET)
endif()

find_path(HEIF_INCLUDE_DIR NAMES libheif/heif.h
                           PATHS ${PC_HEIF_INCLUDEDIR})
find_library(HEIF_LIBRARY NAMES heif libheif
                          PATHS ${PC_HEIF_LIBDIR})

set(HEIF_VERSION ${PC_HEIF_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HEIF
                                  REQUIRED_VARS HEIF_LIBRARY HEIF_INCLUDE_DIR
                                  VERSION_VAR HEIF_VERSION)

if(HEIF_FOUND)
  set(HEIF_LIBRARIES ${HEIF_LIBRARY})

  if(NOT TARGET HEIF::HEIF)
    add_library(HEIF::HEIF UNKNOWN IMPORTED)
    set_target_properties(HEIF::HEIF PROPERTIES
                                     IMPORTED_LOCATION "${HEIF_LIBRARY}")
  endif()
endif()

mark_as_advanced(HEIF_INCLUDE_DIR HEIF_LIBRARY)
