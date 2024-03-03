aux_source_directory(. GLOBAL_SRC_LIST)
add_library(global STATIC ${GLOBAL_SRC_LIST})
target_link_libraries(global layer base)