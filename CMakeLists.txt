cmake_minimum_required(VERSION 3.11)
project(nawa VERSION 0.5)

set(CMAKE_CXX_STANDARD 17)

# version
set(NAWA_VERSION_MAJOR 0)
set(NAWA_VERSION_MINOR 5)

# build options
option(BuildTests "Build tests" OFF)
option(BuildExamples "Build examples" ON)
option(BuildDocs "Build docs" ON)

# only unix-based OS are supported
if(NOT UNIX)
    message(FATAL_ERROR "Unsupported operating system")
endif()

# configure the file that we need for version checking
configure_file(${PROJECT_SOURCE_DIR}/include/Application.h.in
        ${PROJECT_BINARY_DIR}/include/nawa/Application.h)

# ... SysException.h Types/Compound.h Connection.cpp ...
file(GLOB BlowfishCFiles libs/libbcrypt/crypt_blowfish/*.c)
add_library(bcrypt STATIC libs/libbcrypt/bcrypt.c ${BlowfishCFiles})

# dependencies
include(FetchContent)
FetchContent_Declare(
        fastcgilite
        GIT_REPOSITORY https://github.com/jatofg/fastcgipp.git
        GIT_TAG v3.1.2
)
FetchContent_MakeAvailable(fastcgilite)

add_executable(nawarun
        include/nawa/Request.h
        include/nawa/SysException.h
        include/nawa/Connection.h
        include/nawa/Encoding.h
        include/nawa/Utils.h
        include/nawa/Session.h
        include/nawa/Config.h
        include/nawa/Crypto.h
        include/nawa/UserException.h
        include/nawa/Cookie.h
        include/nawa/Log.h
        include/nawa/AccessFilterList.h
        include/nawa/AppInit.h
        include/nawa/SmtpMailer.h
        include/nawa/Email.h
        include/nawa/File.h
        include/nawa/Engines/HashingEngine.h
        include/nawa/Engines/Argon2HashingEngine.h
        include/nawa/Engines/BcryptHashingEngine.h
        include/nawa/Engines/HashTypeTable.h
        include/nawa/Engines/DefaultHashTypeTable.h
        include/nawa/RequestHandlers/RequestHandler.h
        include/nawa/RequestHandlers/FastcgiRequestHandler.h

        src/main.cpp
        src/Request.cpp
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
        src/RequestHandlers/RequestHandler.cpp
        src/RequestHandlers/FastcgiRequestHandler.cpp

        libs/inih/ini.h
        libs/inih/ini.c

        libs/base64/base64.h
        libs/base64/base64.cpp)

target_link_libraries(nawarun -ldl -lcrypto -lcurl -largon2 bcrypt fastcgilite)
target_include_directories(nawarun PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${fastcgilite_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${fastcgilite_SOURCE_DIR}/include>)

if(BuildTests)
    add_library(assertions SHARED
            tests/assertions.cpp)
    target_include_directories(assertions PUBLIC
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

    add_library(nawatest SHARED
            tests/nawatest.cpp)
    target_include_directories(nawatest PUBLIC
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

    add_library(sessiontest SHARED
            tests/sessiontest.cpp)
    target_include_directories(sessiontest PUBLIC
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

    add_library(emailtest SHARED
            tests/emailtest.cpp)
    target_include_directories(emailtest PUBLIC
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
endif()

if(BuildExamples)
    add_library(helloworld SHARED
            examples/helloworld.cpp)
    target_include_directories(helloworld PUBLIC
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

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
install(TARGETS nawarun
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT Runtime)
install(FILES "${PROJECT_SOURCE_DIR}/config.ini"
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/nawa
        COMPONENT Runtime)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/nawa"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT Development)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/nawa/Application.h"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nawa
        COMPONENT Development)
if(EXISTS /etc/systemd/system)
    install(FILES "${PROJECT_SOURCE_DIR}/systemd/nawa@.service"
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