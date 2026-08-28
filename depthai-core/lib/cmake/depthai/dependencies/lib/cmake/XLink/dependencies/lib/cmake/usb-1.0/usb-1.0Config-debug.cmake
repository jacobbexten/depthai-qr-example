#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "usb-1.0" for configuration "Debug"
set_property(TARGET usb-1.0 APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(usb-1.0 PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libusb-1.0d.so"
  IMPORTED_SONAME_DEBUG "libusb-1.0d.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS usb-1.0 )
list(APPEND _IMPORT_CHECK_FILES_FOR_usb-1.0 "${_IMPORT_PREFIX}/lib/libusb-1.0d.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
