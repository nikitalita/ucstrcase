
# ensure that the go toolchain is available
find_program(GO go)
if(NOT GO)
  message(FATAL_ERROR "go toolchain not found")
endif()

#get the PROJECT binary dir
get_target_property(PROJECT_BINARY_DIR ${PROJECT_NAME} BINARY_DIR)

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/gentables
    COMMAND ${GO} clean -modcache && ${GO} build -o ${PROJECT_BINARY_DIR}/gentables
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building gentables"
    VERBATIM
    USES_TERMINAL
    DEPENDS gen
)
add_custom_target(gentables
    DEPENDS ${PROJECT_BINARY_DIR}/gentables
)

set(GOBIN ${CMAKE_CURRENT_BINARY_DIR})
