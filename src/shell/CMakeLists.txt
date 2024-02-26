
set(LIBSHELL_VERSION 1.0.1)


# Convert GNU Make assignments into CMake ones.
function(transform_makefile_inc INPUT_FILE OUTPUT_FILE)
  file(READ ${INPUT_FILE} MAKEFILE_INC_CMAKE)

  string(REGEX REPLACE "\\\\\n" "" MAKEFILE_INC_CMAKE ${MAKEFILE_INC_CMAKE})
  string(REGEX REPLACE "([A-Za-z_]+) *= *([^\n]*)" "set(\\1 \\2)" MAKEFILE_INC_CMAKE ${MAKEFILE_INC_CMAKE})

  file(WRITE ${OUTPUT_FILE} ${MAKEFILE_INC_CMAKE})
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${INPUT_FILE}")
endfunction()

include_directories(${PROJECT_SOURCE_DIR}/include)

TRANSFORM_MAKEFILE_INC("Makefile.inc", "${CMAKE_CURRENT_BINARY_DIR}/Makefile.inc.cmake")
include("${CMAKE_CURRET_BINARY_DIR}/Makefile.inc.cmake")
set(SOURCES ${CSOURCES})

add_library()