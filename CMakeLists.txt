
include_directories(
    "${workspace_dir}/"
)
file(GLOB_RECURSE maket_source
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(marketlib STATIC ${maket_source})
target_link_libraries(marketlib 
    thostmduserapi_se_6.3.15
    thosttraderapi_se_6.3.15
    commonlib
    mysqlclient
    zmq
    libgo
    protobuf
 )