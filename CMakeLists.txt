cmake_minimum_required(VERSION 3.12)
project(qsf)

set(CMAKE_CXX_STANDARD 14)

# ... SysException.h Types/Compound.h Connection.cpp ...
# ... Application.h
file(GLOB BlowfishCFiles libs/libbcrypt/crypt_blowfish/*.c)
add_library(bcrypt STATIC libs/libbcrypt/bcrypt.c ${BlowfishCFiles})
add_executable(qsf src/main.cpp src/Request.cpp include/qsf/Request.h src/RequestHandler.cpp include/qsf/RequestHandler.h include/qsf/SysException.h include/qsf/Types/Compound.h src/Connection.cpp include/qsf/Connection.h src/Encoding.cpp include/qsf/Encoding.h src/Utils.cpp include/qsf/Utils.h src/Session.cpp include/qsf/Session.h libs/inih/ini.c libs/inih/ini.h src/Config.cpp include/qsf/Config.h src/Crypto.cpp include/qsf/Crypto.h include/qsf/UserException.h include/qsf/Types/Universal.h include/qsf/Cookie.h src/Log.cpp include/qsf/Log.h)
target_link_libraries(qsf -ldl -lfastcgipp -lcrypto ${CMAKE_BINARY_DIR}/libbcrypt.a)
target_include_directories(qsf PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

set(CMAKE_CXX_FLAGS "-Wl,--export-dynamic")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--export-dynamic")