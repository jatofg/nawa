cmake_minimum_required(VERSION 3.12)
project(qsf)

set(CMAKE_CXX_STANDARD 14)

# ... SysException.h Types/Compound.h Connection.cpp ...
# ... Application.h
add_executable(qsf main.cpp Request.cpp Request.h RequestHandler.cpp RequestHandler.h SysException.cpp SysException.h Types/Compound.h Connection.cpp Connection.h Encoding.cpp Encoding.h Utils.cpp Utils.h Session.cpp Session.h inih/ini.c inih/ini.h Config.cpp Config.h Crypto.cpp Crypto.h UserException.cpp UserException.h)
target_link_libraries(qsf -ldl -lfastcgipp -lcrypto)

set(CMAKE_CXX_FLAGS "-Wl,--export-dynamic")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--export-dynamic")