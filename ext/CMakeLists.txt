add_subdirectory(etl)
add_subdirectory(ulog/src)
add_subdirectory(cpputest)

add_library(zcbor EXCLUDE_FROM_ALL
zcbor/src/zcbor_common.c
zcbor/src/zcbor_decode.c
zcbor/src/zcbor_encode.c
)
target_include_directories(zcbor PUBLIC zcbor/include)

add_subdirectory(json)
add_subdirectory(crow)
add_subdirectory(spdlog)
add_subdirectory(cli)
