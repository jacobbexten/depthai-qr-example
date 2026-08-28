#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "XLink" for configuration "Debug"
set_property(TARGET XLink APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(XLink PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C;CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libXLinkd.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS XLink )
list(APPEND _IMPORT_CHECK_FILES_FOR_XLink "${_IMPORT_PREFIX}/lib/libXLinkd.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
