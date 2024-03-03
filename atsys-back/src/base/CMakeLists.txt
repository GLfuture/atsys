aux_source_directory(. BASE_SRC_LIST)
add_library(base STATIC ${BASE_SRC_LIST})
target_link_libraries(base yaml-cpp hiredis ssl crypto)
