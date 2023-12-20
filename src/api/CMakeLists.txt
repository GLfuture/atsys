aux_source_directory(. API_SRC_LIST)
add_library(api STATIC ${API_SRC_LIST})
target_link_libraries(api MySqlPool MySql mysqlclient hiredis smtp_grpc_proto ssl crypto global)
