aux_source_directory(. srcs_list)


include_directories(${CMAKE_CURRENT_SOURCE_DIR})

message("${PROJECT_SOURCE_DIR}")
# add_library(srclib SHARED ${srcs_list})
add_library(marketlib  ${srcs_list})
target_link_libraries(marketlib 

    thostmduserapi_se_6.3.15
    thosttraderapi_se_6.3.15
    timelib
    convertlib
    mysqlclient
 )