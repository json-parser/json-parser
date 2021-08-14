###########################################################
## CMAKE SETUP
###########################################################

cmake_minimum_required(VERSION 3.5)
project(json-parser)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

###########################################################
## LIBRARY
###########################################################

add_library(jsonparser SHARED json.c)
add_library(jsonparser-static STATIC json.c)

# Set shared library propertoes
set_target_properties(jsonparser PROPERTIES PUBLIC_HEADER json.h)
set_target_properties(jsonparser PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(jsonparser m)

# Set shared library version
set(jsonparser_VERSION_MAJOR 1)
set(jsonparser_VERSION_MINOR 1)
set(jsonparser_VERSION_PATCH 0)
set(jsonparser_VERSION
    ${jsonparser_VERSION_MAJOR}.${jsonparser_VERSION_MINOR}.${jsonparser_VERSION_PATCH})
set_target_properties(jsonparser PROPERTIES VERSION ${jsonparser_VERSION})
set_target_properties(jsonparser PROPERTIES SOVERSION ${jsonparser_VERSION_MAJOR})

# CMake packaging framework variables setup
set(CPACK_PACKAGE_NAME jsonparser)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "")
set(CPACK_PACKAGE_VERSION_MAJOR ${jsonparser_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${jsonparser_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${jsonparser_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${jsonparser_VERSION})

# Installation target setup
install(TARGETS jsonparser
  EXPORT jsonparser-targets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jsonparser
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION $${CMAKE_INSTALL_LIBDIR}/cmake/jsonparser)

# Generate pkgconfig file (libjsonparser.pc)
if (OS_WINDOWS)
    set(PKGCONFIG_INSTALL_DIR)
else()
    set(PKGCONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()
if (PKGCONFIG_INSTALL_DIR)
    configure_file("json-parser-cmake.pc.in" "${PROJECT_BINARY_DIR}/lib${CPACK_PACKAGE_NAME}.pc" @ONLY)
    install(FILES "${PROJECT_BINARY_DIR}/lib${CPACK_PACKAGE_NAME}.pc" DESTINATION "${PKGCONFIG_INSTALL_DIR}")
endif()

###########################################################
## EXECUTABLES
###########################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test_json ${CMAKE_CURRENT_SOURCE_DIR}/examples/test_json.c)

target_link_libraries(test_json jsonparser)

###########################################################
## COMPILER FLAGS
###########################################################

# Enable all possible warnings and treat them as errors
set(WRN_FLAGS -Wall -Wextra -Wpedantic -Werror)
target_compile_options(jsonparser PRIVATE ${WRN_FLAGS})
target_compile_options(jsonparser-static PRIVATE ${WRN_FLAGS})
target_compile_options(test_json PRIVATE ${WRN_FLAGS})

# Enable address sanitizer for debug builds
set(SNTZR_FLAGS -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak -fsanitize=undefined)
target_compile_options(jsonparser PRIVATE "$<$<CONFIG:DEBUG>:${SNTZR_FLAGS}>")
target_link_options(jsonparser PRIVATE "$<$<CONFIG:DEBUG>:${SNTZR_FLAGS}>")
target_compile_options(jsonparser-static PRIVATE "$<$<CONFIG:DEBUG>:${SNTZR_FLAGS}>")
target_link_options(jsonparser-static PRIVATE "$<$<CONFIG:DEBUG>:${SNTZR_FLAGS}>")
target_compile_options(test_json PRIVATE "$<$<CONFIG:DEBUG>:${SNTZR_FLAGS}>")
target_link_options(test_json PRIVATE "$<$<CONFIG:DEBUG>:${SNTZR_FLAGS}>")

target_compile_options(jsonparser PUBLIC -O3 -Os -g)
