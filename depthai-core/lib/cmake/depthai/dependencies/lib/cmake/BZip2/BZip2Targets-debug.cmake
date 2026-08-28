#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "BZip2::bz2" for configuration "Debug"
set_property(TARGET BZip2::bz2 APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(BZip2::bz2 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libbz2d.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS BZip2::bz2 )
list(APPEND _IMPORT_CHECK_FILES_FOR_BZip2::bz2 "${_IMPORT_PREFIX}/lib/libbz2d.a" )

# Import target "BZip2::bzip2recover" for configuration "Debug"
set_property(TARGET BZip2::bzip2recover APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(BZip2::bzip2recover PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/bzip2recover"
  )

list(APPEND _IMPORT_CHECK_TARGETS BZip2::bzip2recover )
list(APPEND _IMPORT_CHECK_FILES_FOR_BZip2::bzip2recover "${_IMPORT_PREFIX}/bin/bzip2recover" )

# Import target "BZip2::bzip2_bin" for configuration "Debug"
set_property(TARGET BZip2::bzip2_bin APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(BZip2::bzip2_bin PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/bzip2"
  )

list(APPEND _IMPORT_CHECK_TARGETS BZip2::bzip2_bin )
list(APPEND _IMPORT_CHECK_FILES_FOR_BZip2::bzip2_bin "${_IMPORT_PREFIX}/bin/bzip2" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
