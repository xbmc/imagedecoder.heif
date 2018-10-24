#.rst:
# Findde265
# --------
# Finds the de265 library
#
# This will will define the following variables::
#
# de265_FOUND - system has de265
# de265_INCLUDE_DIRS - the de265 include directory
# de265_LIBRARIES - the de265 libraries
#
# and the following imported targets::
#
#   DE265::DE265- The de265 library

if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_DE265 libde265 QUIET)
endif()

find_path(de265_INCLUDE_DIR NAMES libde265/de265.h
                            PATHS ${PC_DE265_INCLUDEDIR})
find_library(de265_LIBRARY NAMES de265 libde265
                          PATHS ${PC_DE265_LIBDIR})

set(de265_VERSION ${PC_DE265_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(de265
                                  REQUIRED_VARS de265_LIBRARY de265_INCLUDE_DIR
                                  VERSION_VAR de265_VERSION)

if(de265_FOUND)
  set(de265_LIBRARIES ${de265_LIBRARY})

  if(NOT TARGET DE265::DE265)
    add_library(DE265::DE265 UNKNOWN IMPORTED)
    set_target_properties(DE265::DE265 PROPERTIES
                                       IMPORTED_LOCATION "${de265_LIBRARY}")
  endif()
endif()

mark_as_advanced(de265_INCLUDE_DIR de265_LIBRARY)
