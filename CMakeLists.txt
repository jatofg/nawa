cmake_minimum_required(VERSION 3.12)
project(qsf)

set(CMAKE_CXX_STANDARD 14)

# ... Exception.h Types/Compound.h Response.cpp ...
# ... Application.h
add_executable(qsf main.cpp Request.cpp Request.h RequestHandler.cpp RequestHandler.h Exception.cpp Exception.h Response.cpp Response.h)
target_link_libraries(qsf -ldl -lfastcgipp)

set(CMAKE_CXX_FLAGS "-Wl,--export-dynamic")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--export-dynamic")