cmake_minimum_required(VERSION 3.7)
project(qsf VERSION 0.1)

set(CMAKE_CXX_STANDARD 14)

# version
set(QSF_VERSION_MAJOR 0)
set(QSF_VERSION_MINOR 1)

# only unix-based OS are supported
if(NOT UNIX)
    message(FATAL_ERROR "Unsupported operating system")
endif()

# configure the file that we need for version checking
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
        include/qsf/AccessFilterList.h
        include/qsf/AppInit.h
        include/qsf/SmtpMailer.h
        include/qsf/Email.h
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
        src/Email.cpp
        src/SmtpMailer.cpp

        libs/inih/ini.h
        libs/inih/ini.c

        libs/base64/base64.h
        libs/base64/base64.cpp)

target_link_libraries(qsf -ldl -lfastcgipp -lcrypto -lcurl ${CMAKE_BINARY_DIR}/libbcrypt.a)
target_include_directories(qsf PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

add_library(qsftest SHARED
        tests/app.h
        tests/qsftest.cpp)
target_include_directories(qsftest PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

add_library(sessiontest SHARED
        tests/app.h
        tests/sessiontest.cpp)
target_include_directories(sessiontest PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

add_library(emailtest SHARED
        tests/app.h
        tests/emailtest.cpp)
target_include_directories(emailtest PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

set(CMAKE_CXX_FLAGS "-Wl,--export-dynamic")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--export-dynamic")

# install targets, components should be checked again w.r.t. packaging
include(GNUInstallDirs)
install(TARGETS qsf
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT Runtime)
install(FILES "${PROJECT_SOURCE_DIR}/config.ini"
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/qsf
        COMPONENT Runtime)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/qsf"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT Development)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/qsf/Application.h"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qsf
        COMPONENT Development)

# build documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in"
            "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" @ONLY)
    add_custom_target(docs
            "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            COMMENT "Generating Doxygen documentation" VERBATIM)
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/docs"
            DESTINATION ${CMAKE_INSTALL_DOCDIR}
            OPTIONAL)
endif(DOXYGEN_FOUND)