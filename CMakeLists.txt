
include_directories(
    "${workspace_dir}/"
)
file(GLOB_RECURSE maket_source
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
)

add_library(marketlib STATIC ${maket_source})
target_link_libraries(marketlib 
    thostmduserapi_se_6.5.1
    xtpquoteapi
    btpmarketapi
    ftpmarketapi
    oes_api
    commonlib
    zmq
    protobuf
    python3.8
    sqlite3
    z
    curl
 )