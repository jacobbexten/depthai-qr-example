cmake_minimum_required(VERSION 3.10.2)
project("qrtest")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(APP_MAIN_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(JNI_LIB_DIR "${APP_MAIN_DIR}/jniLibs/${ANDROID_ABI}")

set(DEPTHAI_INSTALL "${CMAKE_CURRENT_LIST_DIR}/../../../depthai-core")
set(DEPTHAI_INCLUDE_DIR "${DEPTHAI_INSTALL}/include")
set(DEPTHAI_DEPS_INCLUDE_DIR "${DEPTHAI_INSTALL}/lib/cmake/depthai/dependencies/include")
set(DEPTHAI_3RDPARTY_INCLUDE_DIR "${DEPTHAI_INSTALL}/include/depthai-shared/3rdparty")

add_library(libusb-lib SHARED IMPORTED)
set_target_properties(libusb-lib PROPERTIES
    IMPORTED_LOCATION "${JNI_LIB_DIR}/libusb-1.0.so"
)

add_library(depthai-lib SHARED IMPORTED)
set_target_properties(depthai-lib PROPERTIES
    IMPORTED_LOCATION "${JNI_LIB_DIR}/libdepthai-core.so"
)

# Keep only if you still need OpenCV in native code
add_library(opencv-lib SHARED IMPORTED)
set_target_properties(opencv-lib PROPERTIES
    IMPORTED_LOCATION "${JNI_LIB_DIR}/libopencv_java4.so"
)

add_library(qrtest SHARED
    cpp/native-lib.cpp
)

target_compile_features(qrtest PRIVATE cxx_std_17)

target_include_directories(qrtest PRIVATE
    "${APP_MAIN_DIR}/cpp"
    "${DEPTHAI_INCLUDE_DIR}"
    "${DEPTHAI_DEPS_INCLUDE_DIR}"
    "${DEPTHAI_3RDPARTY_INCLUDE_DIR}"
)

find_library(log-lib log)
find_library(android-lib android)

target_link_libraries(qrtest
    PRIVATE
    depthai-lib
    libusb-lib
    opencv-lib
    ${android-lib}
    ${log-lib}
)