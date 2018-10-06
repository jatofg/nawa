cmake_minimum_required(VERSION 3.12)
project(qsf)

set(CMAKE_CXX_STANDARD 14)

add_executable(qsf main.cpp Request.cpp Request.h RequestHandler.cpp RequestHandler.h Exception.cpp Exception.h)
target_link_libraries(qsf -lfastcgipp)