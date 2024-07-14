find_package(simdutf CONFIG REQUIRED)

add_library(simdutf-wrapper STATIC
  src/simdutf-wrapper.cpp

)
set_target_properties(simdutf-wrapper PROPERTIES CXX_STANDARD 17)

# set the language
set_target_properties(simdutf-wrapper PROPERTIES LINKER_LANGUAGE CXX)


target_link_libraries(simdutf-wrapper PRIVATE simdutf::simdutf)
target_include_directories(simdutf-wrapper PUBLIC include)

# print out the include directories
get_target_property(simdutf-wrapper_INCLUDE_DIRS simdutf-wrapper INCLUDE_DIRECTORIES)
message(STATUS "simdutf-wrapper_INCLUDE_DIRS: ${simdutf-wrapper_INCLUDE_DIRS}")