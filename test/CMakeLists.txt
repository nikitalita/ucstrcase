set(TEST_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/test ${PROJECT_SOURCE_DIR}/simdutf-wrapper/include)
add_executable(test_thingy testthingy.c)

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
find_package(fmt REQUIRED)
find_package(simdutf CONFIG REQUIRED)
find_package(catch2 CONFIG REQUIRED)
set(CATCH_CONFIG_CONSOLE_WIDTH "800")
set(CATCH_CONFIG_COLOUR_ANSI "ON")
add_dependencies(test_thingy simdutf-wrapper)


add_executable(norm_tests run_norm_tests.cpp normalization_tests.cpp test_stuff.cpp)
add_dependencies(norm_tests ucstrcase)
target_include_directories(norm_tests PRIVATE ${TEST_INCLUDE_DIRS})
target_link_libraries(norm_tests PRIVATE ucstrcase simdutf-wrapper Catch2::Catch2 Catch2::Catch2WithMain)
set_target_properties(norm_tests PROPERTIES CXX_STANDARD 17)


# turn on the time report
if (EMIT_TIME_REPORT)
  target_compile_options(norm_tests PRIVATE -ftime-report)
endif()
find_package(PkgConfig REQUIRED)
pkg_check_modules(ICU REQUIRED IMPORTED_TARGET icu-uc icu-i18n icu-io)

# build it as a c17 program
set_target_properties(test_thingy PROPERTIES C_STANDARD 17)
set_target_properties(test_thingy PROPERTIES C_STANDARD_REQUIRED ON)
# project include directory
target_link_libraries(test_thingy PRIVATE ucstrcase simdutf-wrapper)
target_include_directories(test_thingy PRIVATE ${TEST_INCLUDE_DIRS})

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


add_executable(utf8_bench bench.cpp thirdparty/utf8proc.c thirdparty/utf8proc_data.c test_stuff.cpp)
# add_dependencies(utf8_bench ucstrcase)
# add_sanitizer(utf8_bench)

set_target_properties(utf8_bench PROPERTIES CXX_STANDARD 23)
set_target_properties(utf8_bench PROPERTIES CXX_STANDARD_REQUIRED ON)
# get the project source path
target_include_directories(utf8_bench PRIVATE ${TEST_INCLUDE_DIRS})
target_compile_options(utf8_bench PRIVATE ${ICU_CFLAGS} -fexperimental-library)
#Add the build variant type to the compile options
target_compile_options(utf8_bench PRIVATE $<$<CONFIG:Debug>:-DBUILT_WITH_DEBUG>)


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
endif()
target_link_options(utf8_bench PRIVATE ${ICU_LDFLAGS})
target_link_libraries(utf8_bench ${ICU_LIBRARIES} ${PROJECT_NAME} fmt::fmt simdutf::simdutf simdutf-wrapper)

# print out the compile options, the linked libraries, and the linked options
get_target_property(utf8_bench_INCLUDE_DIRS utf8_bench INCLUDE_DIRECTORIES)
get_target_property(utf8_bench_COMPILE_OPTIONS utf8_bench COMPILE_OPTIONS)
get_target_property(utf8_bench_LINK_LIBRARIES utf8_bench LINK_LIBRARIES)
get_target_property(utf8_bench_LINK_OPTIONS utf8_bench LINK_OPTIONS)
message(STATUS "utf8_bench_INCLUDE_DIRS: ${utf8_bench_INCLUDE_DIRS}")
message(STATUS "utf8_bench_COMPILE_OPTIONS: ${utf8_bench_COMPILE_OPTIONS}")
message(STATUS "utf8_bench_LINK_LIBRARIES: ${utf8_bench_LINK_LIBRARIES}")
message(STATUS "utf8_bench_LINK_OPTIONS: ${utf8_bench_LINK_OPTIONS}")

add_test(NAME utf8_bench COMMAND utf8_bench)
add_test(NAME norm_tests COMMAND norm_tests)
include(CTest)
