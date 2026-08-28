#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ZLIB::zlib" for configuration "Debug"
set_property(TARGET ZLIB::zlib APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(ZLIB::zlib PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libzd.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS ZLIB::zlib )
list(APPEND _IMPORT_CHECK_FILES_FOR_ZLIB::zlib "${_IMPORT_PREFIX}/lib/libzd.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
