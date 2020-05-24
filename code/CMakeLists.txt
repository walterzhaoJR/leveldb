cmake_minimum_required(VERSION 3.9)

include_directories(../include)
link_directories(../build)

project(leveldb_test_demo)

add_executable(main main.cpp)
target_link_libraries(main libleveldb.a)
target_link_libraries(main pthread)
