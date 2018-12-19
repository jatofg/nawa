cmake_minimum_required(VERSION 3.12)
project(qsf)

set(CMAKE_CXX_STANDARD 14)

# ... SysException.h Types/Compound.h Connection.cpp ...
# ... Application.h
file(GLOB BlowfishCFiles libbcrypt/crypt_blowfish/*.c)
add_library(bcrypt STATIC libbcrypt/bcrypt.c ${BlowfishCFiles} libbcrypt/crypt_blowfish/x86.S)
add_executable(qsf main.cpp Request.cpp Request.h RequestHandler.cpp RequestHandler.h SysException.h Types/Compound.h Connection.cpp Connection.h Encoding.cpp Encoding.h Utils.cpp Utils.h Session.cpp Session.h inih/ini.c inih/ini.h Config.cpp Config.h Crypto.cpp Crypto.h UserException.h Types/Universal.h)
target_link_libraries(qsf -ldl -lfastcgipp -lcrypto ${CMAKE_BINARY_DIR}/libbcrypt.a)

set(CMAKE_CXX_FLAGS "-Wl,--export-dynamic")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--export-dynamic")