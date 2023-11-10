find_package(cpp-jwt REQUIRED)
aux_source_directory(. API_SRC_LIST)
add_library(api STATIC ${API_SRC_LIST})
target_link_libraries(api MySqlPool MySql mysqlclient hiredis cpp-jwt::cpp-jwt)