
# ensure that the go toolchain is available
find_program(GO go)
if(NOT GO)
  message(FATAL_ERROR "go toolchain not found")
endif()

#get the PROJECT binary dir
get_target_property(PROJECT_BINARY_DIR ${PROJECT_NAME} BINARY_DIR)
get_target_property(PROJECT_SOURCE_DIR ${PROJECT_NAME} SOURCE_DIR)
add_custom_target(gen
    COMMAND ${GO} clean -modcache && ${GO} build -o ${PROJECT_BINARY_DIR}/gen
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building gen"
    USES_TERMINAL
    VERBATIM
)

add_custom_target(gentables
    COMMAND cd gentables && ${GO} clean -modcache && ${GO} build -o ${PROJECT_BINARY_DIR}/gentables
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building gentables"
    VERBATIM
    USES_TERMINAL
    DEPENDS gen
)
#
##check if ${UCSTRCASE_UNICODE_VERSION} is set
#if(NOT DEFINED UCSTRCASE_UNICODE_VERSION)
#  message(FATAL_ERROR "UCSTRCASE_UNICODE_VERSION is not defined")
#endif()
#if (NOT UCSTRCASE_UNICODE_VERSION MATCHES "([0-9]+)\\.([0-9]+)\\.([0-9]+)")
#  message(FATAL_ERROR "UCSTRCASE_UNICODE_VERSION is not in the format x.y.z")
#endif()
#if (NOT UCSTRCASE_CLDR_VERSION MATCHES "([0-9]+)")
#  message(FATAL_ERROR "UCSTRCASE_CLDR_VERSION is not in the format x")
#endif()
#make sure that it outputs to console
add_custom_target(run_gentables
    COMMAND ${PROJECT_BINARY_DIR}/gentables -dir ${PROJECT_SOURCE_DIR} #-unicode ${UCSTRCASE_UNICODE_VERSION} -cldr ${UCSTRCASE_CLDR_VERSION}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running gentables"
    VERBATIM
    USES_TERMINAL
    DEPENDS gentables
)

set(GOBIN ${CMAKE_CURRENT_BINARY_DIR})
