cmake_minimum_required(VERSION 3.0)

add_executable(reflector2 ast.cc)

target_include_directories(reflector2 PRIVATE json/src)
target_link_libraries(reflector2 PUBLIC clang)
