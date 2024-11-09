
include_directories(
    "${workspace_dir}/"
)
file(GLOB_RECURSE maket_source
    "${CMAKE_CURRENT_SOURCE_DIR}/domain/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/entry/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/infra/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/interface/*.cc"
)

add_library(marketlib STATIC ${maket_source})
target_link_libraries(marketlib 
    thostmduserapi_se_6.5.1
    xtpquoteapi
    btpmarketapi
    ftpmarketapi
    gtpmarketapi
    mtpmarketapi
    oes_api
    commonlib
    zmq
    protobuf
    python3.8
    sqlite3
    quickfix
    z
    curl
    boost_system
    dl
)

file(GLOB_RECURSE test_source
    "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cc"
)

set(shared_libs pthread gmock gtest gtest_main gmock_main marketlib)

foreach(test_source ${test_source})
    get_filename_component(test_name ${test_source} NAME_WE)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} ${shared_libs})
endforeach()
