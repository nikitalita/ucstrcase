add_subdirectory(gen)
get_target_property(PROJECT_SOURCE_DIR ${PROJECT_NAME} SOURCE_DIR)
#
##check if ${UCSTRCASE_UNICODE_VERSION} is set
if(NOT DEFINED UCSTRCASE_UNICODE_VERSION)
    message(FATAL_ERROR "UCSTRCASE_UNICODE_VERSION is not defined")
endif()
if (NOT UCSTRCASE_UNICODE_VERSION MATCHES "([0-9]+)\\.([0-9]+)\\.([0-9]+)")
    message(FATAL_ERROR "UCSTRCASE_UNICODE_VERSION is not in the format x.y.z")
endif()
if (NOT UCSTRCASE_CLDR_VERSION MATCHES "([0-9]+)")
    message(FATAL_ERROR "UCSTRCASE_CLDR_VERSION is not in the format x")
endif()
   
add_custom_target(run_gentables
    # TODO: Turn this back on after this refactor
    COMMAND ${PROJECT_BINARY_DIR}/gentables -dir ${PROJECT_SOURCE_DIR} #-unicode ${UCSTRCASE_UNICODE_VERSION} -cldr ${UCSTRCASE_CLDR_VERSION}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/gen
    COMMENT "Running gentables"
    VERBATIM
    USES_TERMINAL
    DEPENDS gentables
)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_custom_target(
#          OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/src/normtables.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/normtables.c" "${CMAKE_CURRENT_SOURCE_DIR}/test/normalization_tests.h" "${CMAKE_CURRENT_SOURCE_DIR}/test/normalization_tests.c"
        generate_normtables
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/unicode.py" --unicode ${UCSTRCASE_UNICODE_VERSION}
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        COMMENT "Generating normalization tables..."
        USES_TERMINAL
)
