# Proto file
get_filename_component(smtp_proto "smtp.proto" ABSOLUTE)
get_filename_component(smtp_proto_path "${smtp_proto}" PATH)

# Generated sources
set(smtp_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/smtp.pb.cc")
set(smtp_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/smtp.pb.h")
set(smtp_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/smtp.grpc.pb.cc")
set(smtp_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/smtp.grpc.pb.h")
add_custom_command(
      OUTPUT "${smtp_proto_srcs}" "${smtp_proto_hdrs}" "${smtp_grpc_srcs}" "${smtp_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${smtp_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${smtp_proto}"
      DEPENDS "${smtp_proto}")

# smtp_grpc_proto
add_library(smtp_grpc_proto
  ${smtp_grpc_srcs}
  ${smtp_grpc_hdrs}
  ${smtp_proto_srcs}
  ${smtp_proto_hdrs})
target_link_libraries(smtp_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# Targets greeter_[async_](client|server)
add_executable(smtp_server smtp_server.cc base64.cc)
target_link_libraries(smtp_server
    smtp_grpc_proto
    spdlog
    base
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})



