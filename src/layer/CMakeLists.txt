aux_source_directory(. LAYER_SRC_LIST)
add_library(layer STATIC ${LAYER_SRC_LIST})
target_link_libraries(layer api pthread base)