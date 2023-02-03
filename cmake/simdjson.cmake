set(SIMDJSON_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/contribs/simdjson/include")
set(SIMDJSON_SRC_DIR "${CMAKE_SOURCE_DIR}/contribs/simdjson/src")
set(SIMDJSON_SRC "${SIMDJSON_SRC_DIR}/simdjson.cpp")
add_library(_simdjson ${SIMDJSON_SRC})
target_include_directories(_simdjson SYSTEM PUBLIC "${SIMDJSON_INCLUDE_DIR}" PRIVATE "${SIMDJSON_SRC_DIR}")

# simdjson is using its own CPU dispatching and get confused if we enable AVX/AVX2 flags.
if(ARCH_AMD64)
    target_compile_options(_simdjson PRIVATE -mno-avx -mno-avx2)
endif()

add_library(contrib::simdjson ALIAS _simdjson)