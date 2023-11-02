cmake_minimum_required(VERSION 3.0)
project(atsys)
set(CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_INSTALL_PREFIX ../..)

set(SRC_LIST main.cc)
set(LIBS spdlog api app layer global MySqlPool MySql mysqlclient)
link_directories(../lib)
include_directories(../include)

add_subdirectory(./api)
add_subdirectory(./app)
add_subdirectory(./base)
add_subdirectory(./layer)
add_subdirectory(./global)

add_executable(atsys ${SRC_LIST})

target_link_libraries(atsys ${LIBS})
install(TARGETS atsys DESTINATION sbin)
