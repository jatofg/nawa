cmake_minimum_required(VERSION 3.7)
project(soru VERSION 0.1)

set(CMAKE_CXX_STANDARD 14)

# version
set(SORU_VERSION_MAJOR 0)
set(SORU_VERSION_MINOR 1)

# build options
option(BuildTests "Build tests" ON)
option(BuildExamples "Build examples" ON)
option(BuildDocs "Build docs" ON)

# only unix-based OS are supported
if(NOT UNIX)
    message(FATAL_ERROR "Unsupported operating system")
endif()

# configure the file that we need for version checking
configure_file(${PROJECT_SOURCE_DIR}/include/Application.h.in
        ${PROJECT_BINARY_DIR}/include/soru/Application.h)

# ... SysException.h Types/Compound.h Connection.cpp ...
file(GLOB BlowfishCFiles libs/libbcrypt/crypt_blowfish/*.c)
add_library(bcrypt STATIC libs/libbcrypt/bcrypt.c ${BlowfishCFiles})

add_executable(sorun
        include/soru/Request.h
        include/soru/RequestHandler.h
        include/soru/SysException.h
        include/soru/Connection.h
        include/soru/Encoding.h
        include/soru/Utils.h
        include/soru/Session.h
        include/soru/Config.h
        include/soru/Crypto.h
        include/soru/UserException.h
        include/soru/Cookie.h
        include/soru/Log.h
        include/soru/AccessFilterList.h
        include/soru/AppInit.h
        include/soru/SmtpMailer.h
        include/soru/Email.h
        include/soru/Universal.h
        include/soru/Engines/HashingEngine.h
        include/soru/Engines/Argon2HashingEngine.h
        include/soru/Engines/BcryptHashingEngine.h
        include/soru/Engines/HashTypeTable.h
        include/soru/Engines/DefaultHashTypeTable.h

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
        src/Engines/Argon2HashingEngine.cpp
        src/Engines/BcryptHashingEngine.cpp
        src/Engines/DefaultHashTypeTable.cpp

        libs/inih/ini.h
        libs/inih/ini.c

        libs/base64/base64.h
        libs/base64/base64.cpp)

target_link_libraries(sorun -ldl -lfastcgipp -lcrypto -lcurl -largon2 bcrypt)
target_include_directories(sorun PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

if(BuildTests)
    add_library(sorutest SHARED
            tests/app.h
            tests/sorutest.cpp)
    target_include_directories(sorutest PUBLIC
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
endif()

if(BuildExamples)
    add_library(contactform SHARED
            examples/contactform.cpp)
    target_include_directories(contactform PUBLIC
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

    add_library(multipage SHARED
            examples/multipage.cpp)
    target_include_directories(multipage PUBLIC
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
endif()

set(CMAKE_CXX_FLAGS "-Wl,--export-dynamic")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--export-dynamic")

# install targets, components should be checked again w.r.t. packaging
include(GNUInstallDirs)
install(TARGETS sorun
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT Runtime)
install(FILES "${PROJECT_SOURCE_DIR}/config.ini"
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/soru
        COMPONENT Runtime)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/soru"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT Development)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/soru/Application.h"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/soru
        COMPONENT Development)
if(EXISTS /etc/systemd/system)
    install(FILES "${PROJECT_SOURCE_DIR}/systemd/soru@.service"
            DESTINATION /etc/systemd/system
            COMPONENT Runtime)
endif()

# build documentation
find_package(Doxygen)
if(DOXYGEN_FOUND AND BuildDocs)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in"
            "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" @ONLY)
    add_custom_target(docs
            "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            COMMENT "Generating Doxygen documentation" VERBATIM)
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/docs"
            DESTINATION ${CMAKE_INSTALL_DOCDIR}
            OPTIONAL)
endif()