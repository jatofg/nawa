cmake_minimum_required(VERSION 3.12)
project(qsf)

set(CMAKE_CXX_STANDARD 14)

# version
set(QSF_VERSION_MAJOR 0)
set(QSF_VERSION_MINOR 1)

#configure the file that we need for version checking
configure_file(${PROJECT_SOURCE_DIR}/include/Application.h.in
        ${PROJECT_BINARY_DIR}/include/qsf/Application.h)

# ... SysException.h Types/Compound.h Connection.cpp ...
file(GLOB BlowfishCFiles libs/libbcrypt/crypt_blowfish/*.c)
add_library(bcrypt STATIC libs/libbcrypt/bcrypt.c ${BlowfishCFiles})

add_executable(qsf
        include/qsf/Request.h
        include/qsf/RequestHandler.h
        include/qsf/SysException.h
        include/qsf/Connection.h
        include/qsf/Encoding.h
        include/qsf/Utils.h
        include/qsf/Session.h
        include/qsf/Config.h
        include/qsf/Crypto.h
        include/qsf/UserException.h
        include/qsf/Cookie.h
        include/qsf/Log.h
        include/qsf/Types/Compound.h
        include/qsf/Types/Universal.h

        src/main.cpp
        src/Request.cpp
        src/RequestHandler.cpp
        src/Connection.cpp
        src/Encoding.cpp
        src/Utils.cpp
        src/Session.cpp
        src/Config.cpp
        src/Crypto.cpp
        src/Log.cpp

        libs/inih/ini.c
        libs/inih/ini.h)

target_link_libraries(qsf -ldl -lfastcgipp -lcrypto ${CMAKE_BINARY_DIR}/libbcrypt.a)
target_include_directories(qsf PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

set(CMAKE_CXX_FLAGS "-Wl,--export-dynamic")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--export-dynamic")