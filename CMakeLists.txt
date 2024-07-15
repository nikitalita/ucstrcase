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
option(USE_SIMDUTF "Link against simdutf for converting strings" ON)
option(EMIT_TIME_REPORT "Emit the compile time report" OFF)

set(UCSTRCASE_UNICODE_VERSION "15.1.0")
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

add_library(ucstrcase src/ucstrcase.c src/data.c src/compose.c src/normalize.c src/normtables.c src/decode.c src/lookup.c)
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



if (USE_SIMDUTF)
  find_package(simdutf CONFIG REQUIRED)
  add_subdirectory(simdutf-wrapper)
  target_compile_definitions(ucstrcase PRIVATE UCSTRCASE_USE_SIMDUTF)
  target_link_libraries(ucstrcase PRIVATE "$<BUILD_INTERFACE:simdutf-wrapper>")
endif()
#configure the Config.pc.in
configure_file(${UCSTRCASE_CMAKE_PKGCONFIG_TEMPLATE} "${UCSTRCASE_CMAKE_PROJECT_PKGCONFIG_FILE}" @ONLY)
#
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
  add_subdirectory(internal)
  add_dependencies(ucstrcase run_gentables)
  add_dependencies(ucstrcase generate_normtables)
endif()


if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()