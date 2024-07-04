#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ucstrcase::ucstrcase" for configuration "Release"
set_property(TARGET ucstrcase::ucstrcase APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(ucstrcase::ucstrcase PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "/Users/nikita/Workspace/blurgh/install/lib/libucstrcase.a"
  )

list(APPEND _cmake_import_check_targets ucstrcase::ucstrcase )
list(APPEND _cmake_import_check_files_for_ucstrcase::ucstrcase "/Users/nikita/Workspace/blurgh/install/lib/libucstrcase.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
