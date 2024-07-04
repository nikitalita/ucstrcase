# This is free and unencumbered software released into the public domain.
#
# Anyone is free to copy, modify, publish, use, compile, sell, or
# distribute this software, either in source code form or as a compiled
# binary, for any purpose, commercial or non-commercial, and by any
# means.
#
# In jurisdictions that recognize copyright laws, the author or authors
# of this software dedicate any and all copyright interest in the
# software to the public domain. We make this dedication for the benefit
# of the public at large and to the detriment of our heirs and
# successors. We intend this dedication to be an overt act of
# relinquishment in perpetuity of all present and future rights to this
# software under copyright law.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR

#
# For more information, please refer to <http://unlicense.org/>
cmake_minimum_required(VERSION 3.15)
project(ucstrcase VERSION 0.1.0 LANGUAGES CXX C)

# options
set(UTF8_USE_SANITIZER "" CACHE STRING "Set which Clang Sanitizer to use")
option(BUILD_TESTS "Build the tests" OFF)
option(GENERATE_DATA "Generate the data" OFF)


set(UCSTRCASE_UNICODE_VERSION "15.0.0")
set(UCSTRCASE_CLDR_VERSION "32")


macro(add_sanitizer target)
  if(NOT "${UTF8_USE_SANITIZER}" STREQUAL "")
    target_compile_options(${target} PUBLIC -fno-omit-frame-pointer -fsanitize=${UTF8_USE_SANITIZER})
    target_link_options(${target} PUBLIC -fno-omit-frame-pointer -fsanitize=${UTF8_USE_SANITIZER})
  endif()
endmacro()

message("INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")

#check if CMAKE_INSTALL_LIBDIR is defined
if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

#check if CMAKE_INSTALL_INCLUDEDIR is defined
if(NOT DEFINED CMAKE_INSTALL_INCLUDEDIR)
  set(CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include")
endif()

#check if the share directory is defined
if(NOT DEFINED CMAKE_INSTALL_SHAREDIR)
  set(CMAKE_INSTALL_SHAREDIR "${CMAKE_INSTALL_PREFIX}/share")
endif()

set(UCSTRCASE_TARGETS_EXPORT_NAME       "${PROJECT_NAME}-targets")
set(UCSTRCASE_CONFIG_INSTALL_DIR        "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE INTERNAL "")
set(UCSTRCASE_PKGCONFIG_INSTALL_DIR     "${CMAKE_INSTALL_LIBDIR}/pkgconfig" CACHE INTERNAL "")
set(UCSTRCASE_INCLUDE_INSTALL_DIR       "${CMAKE_INSTALL_INCLUDEDIR}/ucstrcase" CACHE INTERNAL "")
set(UCSTRCASE_SHARE_INSTALL_DIR         "${CMAKE_INSTALL_SHAREDIR}/ucstrcase" CACHE INTERNAL "")
set(UCSTRCASE_TARGETS_EXPORT_NAME       "${PROJECT_NAME}-targets")
set(UCSTRCASE_CMAKE_CONFIG_TEMPLATE     "cmake/Config.cmake.in")
set(UCSTRCASE_CMAKE_PKGCONFIG_TEMPLATE  "cmake/Config.pc.in")
set(UCSTRCASE_CMAKE_CONFIG_DIR          "${CMAKE_CURRENT_BINARY_DIR}")
set(UCSTRCASE_CMAKE_VERSION_CONFIG_FILE "${UCSTRCASE_CMAKE_CONFIG_DIR}/${PROJECT_NAME}ConfigVersion.cmake")
set(UCSTRCASE_CMAKE_PROJECT_CONFIG_FILE "${UCSTRCASE_CMAKE_CONFIG_DIR}/${PROJECT_NAME}Config.cmake")
set(UCSTRCASE_CMAKE_PROJECT_TARGETS_FILE "${UCSTRCASE_CMAKE_CONFIG_DIR}/${PROJECT_NAME}-targets.cmake")
set(UCSTRCASE_CMAKE_PROJECT_PKGCONFIG_FILE "${UCSTRCASE_CMAKE_CONFIG_DIR}/${PROJECT_NAME}.pc")

# print all the above strings out
message("UCSTRCASE_TARGETS_EXPORT_NAME: ${UCSTRCASE_TARGETS_EXPORT_NAME}")
message("UCSTRCASE_CONFIG_INSTALL_DIR: ${UCSTRCASE_CONFIG_INSTALL_DIR}")
message("UCSTRCASE_PKGCONFIG_INSTALL_DIR: ${UCSTRCASE_PKGCONFIG_INSTALL_DIR}")
message("UCSTRCASE_INCLUDE_INSTALL_DIR: ${UCSTRCASE_INCLUDE_INSTALL_DIR}")
message("UCSTRCASE_SHARE_INSTALL_DIR: ${UCSTRCASE_SHARE_INSTALL_DIR}")
message("UCSTRCASE_TARGETS_EXPORT_NAME: ${UCSTRCASE_TARGETS_EXPORT_NAME}")
message("UCSTRCASE_CMAKE_CONFIG_TEMPLATE: ${UCSTRCASE_CMAKE_CONFIG_TEMPLATE}")
message("UCSTRCASE_CMAKE_PKGCONFIG_TEMPLATE: ${UCSTRCASE_CMAKE_PKGCONFIG_TEMPLATE}")
message("UCSTRCASE_CMAKE_CONFIG_DIR: ${UCSTRCASE_CMAKE_CONFIG_DIR}")
message("UCSTRCASE_CMAKE_VERSION_CONFIG_FILE: ${UCSTRCASE_CMAKE_VERSION_CONFIG_FILE}")
message("UCSTRCASE_CMAKE_PROJECT_CONFIG_FILE: ${UCSTRCASE_CMAKE_PROJECT_CONFIG_FILE}")
message("UCSTRCASE_CMAKE_PROJECT_TARGETS_FILE: ${UCSTRCASE_CMAKE_PROJECT_TARGETS_FILE}")
message("UCSTRCASE_CMAKE_PROJECT_PKGCONFIG_FILE: ${UCSTRCASE_CMAKE_PROJECT_PKGCONFIG_FILE}")


add_library(ucstrcase src/ucstrcase.c src/data.c)
target_include_directories(ucstrcase PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${UCSTRCASE_INCLUDE_INSTALL_DIR}>)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        ${UCSTRCASE_CMAKE_VERSION_CONFIG_FILE} COMPATIBILITY SameMinorVersion
)
configure_package_config_file(
        ${UCSTRCASE_CMAKE_CONFIG_TEMPLATE}
        "${UCSTRCASE_CMAKE_PROJECT_CONFIG_FILE}"
        INSTALL_DESTINATION ${UCSTRCASE_CONFIG_INSTALL_DIR}
)
#configure the Config.pc.in
configure_file(${UCSTRCASE_CMAKE_PKGCONFIG_TEMPLATE} "${UCSTRCASE_CMAKE_PROJECT_PKGCONFIG_FILE}" @ONLY)

# configure_package_config_file(ucstrcaseConfig.cmake.in ucstrcaseConfig.cmake INSTALL_DESTINATION share/ucstrcase)
# add install target
install(
        TARGETS                  "${PROJECT_NAME}"
        EXPORT                   "${UCSTRCASE_TARGETS_EXPORT_NAME}"
        ARCHIVE DESTINATION      "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION      "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION      "${CMAKE_INSTALL_BINDIR}"
        INCLUDES DESTINATION     "${UCSTRCASE_INCLUDE_INSTALL_DIR}"
)
install(
        EXPORT "${UCSTRCASE_TARGETS_EXPORT_NAME}"
        NAMESPACE "${PROJECT_NAME}::"
        DESTINATION "${UCSTRCASE_CONFIG_INSTALL_DIR}"
)
install(FILES ${UCSTRCASE_CMAKE_VERSION_CONFIG_FILE} ${UCSTRCASE_CMAKE_PROJECT_CONFIG_FILE}
        DESTINATION ${UCSTRCASE_CONFIG_INSTALL_DIR})
# install the pkg-config file
install(FILES ${UCSTRCASE_CMAKE_PROJECT_PKGCONFIG_FILE} DESTINATION "${UCSTRCASE_PKGCONFIG_INSTALL_DIR}")


#add_dependencies(utf8_bench run_gentables)
if (GENERATE_DATA)
  add_subdirectory(internal/gen)
  add_dependencies(ucstrcase run_gentables)
endif()

if(BUILD_TESTS)
  if(VCPKG_TARGET_TRIPLET)
    # clear these out every time or else it won't work when variants change
    set(ICU_LDFLAGS "")
    set(ICU_CFLAGS "")
    set(ICU_LIBRARIES "")
    set(ICU_INCLUDE_DIRS "")
    set(ICU_LDFLAGS_OTHER "")
    set(ICU_CFLAGS_OTHER "")
    set(ICU_FOUND FALSE)  
    # check to see if we have the "test" feature in vcpkg
    if(NOT VCPKG_MANIFEST_FEATURES MATCHES "test")
      message(FATAL_ERROR "Specify \"test\" in VCPKG_MANIFEST_FEATURES to build the tests!")
    endif()
    message(STATUS "VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")
    message(STATUS "CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")

    # pkg-config is a f*cking abomination.
    # check to see if Target triplet includes "windows"
    if(VCPKG_TARGET_TRIPLET MATCHES "windows")
      # check if it's using static-md
      if(NOT VCPKG_TARGET_TRIPLET MATCHES "-static-md")
        message(FATAL_ERROR "\n**************************\nUse the x{32,64}-windows-static-md triplet!\n**************************\n")
      endif()
    endif()
  endif()

  find_package(PkgConfig REQUIRED)
  pkg_check_modules(ICU REQUIRED IMPORTED_TARGET icu-uc icu-i18n icu-io)

  # check if it found it
  if(ICU_FOUND)
    message(STATUS "Found ICU4C: ${ICU_LIBRARIES}")
    message(STATUS "Found ICU4C: ${ICU_INCLUDE_DIRS}")
    message(STATUS "Found ICU4C: ${ICU_LDFLAGS}")
    message(STATUS "Found ICU4C: ${ICU_LDFLAGS_OTHER}")
    message(STATUS "Found ICU4C: ${ICU_CFLAGS}")
    message(STATUS "Found ICU4C: ${ICU_CFLAGS_OTHER}")
  else()
    message(FATAL_ERROR "Could not find ICU4C")
  endif()


  #ensure that gentables is built before the tests

  add_executable(utf8_bench test/bench.cpp)
  # add_dependencies(utf8_bench ucstrcase)
  # add_sanitizer(utf8_bench)
  set_target_properties(utf8_bench PROPERTIES CXX_STANDARD 23)
  set_target_properties(utf8_bench PROPERTIES CXX_STANDARD_REQUIRED ON)
  # project include directory
  target_include_directories(utf8_bench PRIVATE include src)

  target_compile_options(utf8_bench PRIVATE ${ICU_CFLAGS})
  # check if strcasecmp is available
  include( CheckFunctionExists )
  CHECK_FUNCTION_EXISTS(strcasecmp HAVE_STRCASECMP)
  #if not, we have to define strcasecmp as 
  if(NOT HAVE_STRCASECMP)
    target_compile_definitions(utf8_bench PRIVATE -Dstrcasecmp=stricmp)
    target_compile_definitions(utf8_bench PRIVATE -Dstrncasecmp=strnicmp)
  endif()
  
  # target_link_libraries(utf8_bench ${PROJECT_NAME})
  # check if we're using msvc
  if (MSVC)
    #find the -L option in the ICU_LDFLAGS
    string(FIND "${ICU_LDFLAGS}" "-L" ICU_LDFLAGS_POS)
    # get the substring from -L to the end of the string
    string(SUBSTRING "${ICU_LDFLAGS}" "${ICU_LDFLAGS_POS}" -1 ICU_LDFLAGS)
    # find the semicolon after
    string(FIND "${ICU_LDFLAGS}" ";" ICU_LDFLAGS_POS)
    # get the substring from 0 to the semicolon
    string(SUBSTRING "${ICU_LDFLAGS}" 0 "${ICU_LDFLAGS_POS}" ICU_LDFLAGS)
    # replace -L with /LIBPATH:
    string(REPLACE "-L" "/LIBPATH:" ICU_LDFLAGS ${ICU_LDFLAGS})
    target_link_options(utf8_bench PRIVATE ${ICU_LDFLAGS})
  else()
    target_link_options(utf8_bench PRIVATE ${ICU_LDFLAGS})
  endif()
  target_link_libraries(utf8_bench ${ICU_LIBRARIES})
  target_link_libraries(utf8_bench ${PROJECT_NAME})

  # print out the compile options, the linked libraries, and the linked options
  get_target_property(utf8_bench_COMPILE_OPTIONS utf8_bench COMPILE_OPTIONS)
  get_target_property(utf8_bench_LINK_LIBRARIES utf8_bench LINK_LIBRARIES)
  get_target_property(utf8_bench_LINK_OPTIONS utf8_bench LINK_OPTIONS)
  message(STATUS "utf8_bench_COMPILE_OPTIONS: ${utf8_bench_COMPILE_OPTIONS}")
  message(STATUS "utf8_bench_LINK_LIBRARIES: ${utf8_bench_LINK_LIBRARIES}")
  message(STATUS "utf8_bench_LINK_OPTIONS: ${utf8_bench_LINK_OPTIONS}")
endif()