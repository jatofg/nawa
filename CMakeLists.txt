cmake_minimum_required(VERSION 3.13)
project(nawa VERSION 0.7)

set(CMAKE_CXX_STANDARD 17)

# version
set(NAWA_VERSION_MAJOR 0)
set(NAWA_VERSION_MINOR 6)

# build options
option(BuildTests "Build tests" OFF)
option(BuildExamples "Build examples" ON)
option(BuildDocs "Build docs" ON)
option(BuildSharedLib "Build shared library" OFF)
option(EnableArgon2 "Build with argon2 hashing support" ON)

# only unix-based OS are supported
if (NOT UNIX)
    message(FATAL_ERROR "Unsupported operating system")
endif ()

# configure the file that we need for version checking
configure_file(${PROJECT_SOURCE_DIR}/include/Application.h.in
        ${PROJECT_BINARY_DIR}/include/nawa/Application.h)

file(GLOB BlowfishCFiles libs/libbcrypt/crypt_blowfish/*.c)
add_library(bcrypt STATIC libs/libbcrypt/bcrypt.c ${BlowfishCFiles})
target_compile_options(bcrypt PUBLIC -fPIC)

# dependencies
include(FetchContent)
FetchContent_Declare(
        fastcgilite
        GIT_REPOSITORY https://github.com/jatofg/fastcgipp.git
        GIT_TAG v3.9.2
)
FetchContent_Declare(
        cppnetlib
        GIT_REPOSITORY https://github.com/cpp-netlib/cpp-netlib.git
        GIT_TAG 31d304cdf52b485f465ada433d8905171b61cbff
)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v2.13.1
)

# if Debian would ship cmake 3.14 instead of 3.13, we could just do the following
#FetchContent_MakeAvailable(fastcgilite)
FetchContent_GetProperties(fastcgilite)
if (NOT fastcgilite_POPULATED)
    FetchContent_Populate(fastcgilite)
    add_subdirectory(${fastcgilite_SOURCE_DIR} ${fastcgilite_BINARY_DIR})
endif ()

FetchContent_GetProperties(Catch2)
if (NOT catch2_POPULATED)
    FetchContent_Populate(catch2)
    add_subdirectory(${catch2_SOURCE_DIR} ${catch2_BINARY_DIR})
endif ()

FetchContent_GetProperties(cppnetlib)
if (NOT cppnetlib_POPULATED)
    FetchContent_Populate(cppnetlib)
    set(CPP-NETLIB_BUILD_TESTS OFF CACHE INTERNAL "")
    set(CPP-NETLIB_BUILD_EXAMPLES OFF CACHE INTERNAL "")
    add_subdirectory(${cppnetlib_SOURCE_DIR} ${cppnetlib_BINARY_DIR} EXCLUDE_FROM_ALL)
    target_compile_options(cppnetlib-uri PUBLIC -fPIC)
    target_compile_options(cppnetlib-server-parsers PUBLIC -fPIC)
    target_compile_options(cppnetlib-client-connections PUBLIC -fPIC)
endif ()
set(NETLIB_LIBRARIES cppnetlib-client-connections cppnetlib-server-parsers cppnetlib-uri)

set(NAWA_FILES
        include/nawa/Request.h
        include/nawa/Connection.h
        include/nawa/Encoding.h
        include/nawa/Utils.h
        include/nawa/Session.h
        include/nawa/Config.h
        include/nawa/Crypto.h
        include/nawa/Exception.h
        include/nawa/Cookie.h
        include/nawa/Log.h
        include/nawa/AccessFilterList.h
        include/nawa/AppInit.h
        include/nawa/SmtpMailer.h
        include/nawa/Email.h
        include/nawa/File.h
        include/nawa/MimeMultipart.h
        include/nawa/Engines/HashingEngine.h

        include/nawa/Engines/BcryptHashingEngine.h
        include/nawa/Engines/HashTypeTable.h
        include/nawa/Engines/DefaultHashTypeTable.h
        include/nawa/RequestHandlers/RequestHandler.h
        include/nawa/RequestHandlers/FastcgiRequestHandler.h
        include/nawa/RequestHandlers/HttpRequestHandler.h

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
        src/MimeMultipart.cpp

        src/Engines/BcryptHashingEngine.cpp

        src/RequestHandlers/RequestHandler.cpp
        src/RequestHandlers/FastcgiRequestHandler.cpp
        src/RequestHandlers/HttpRequestHandler.cpp

        libs/inih/ini.h
        libs/inih/ini.c

        libs/base64/base64.h
        libs/base64/base64.cpp)

set(NAWA_LINK_LIBRARIES
        -ldl
        -lcrypto
        -lcurl
        -lboost_system
        bcrypt
        fastcgilite
        ${NETLIB_LIBRARIES})

if (EnableArgon2)
    set(NAWA_FILES ${NAWA_FILES}
            include/nawa/Engines/Argon2HashingEngine.h
            src/Engines/Argon2HashingEngine.cpp
            src/Engines/DefaultHashTypeTable.cpp)
    set(NAWA_LINK_LIBRARIES ${NAWA_LINK_LIBRARIES}
            -largon2)
else ()
    set(NAWA_FILES ${NAWA_FILES}
            src/Engines/BcryptOnlyHashTypeTable.cpp)
endif ()

add_executable(nawarun src/main.cpp ${NAWA_FILES})

target_link_libraries(nawarun ${NAWA_LINK_LIBRARIES})
target_include_directories(nawarun PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${fastcgilite_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${fastcgilite_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${cppnetlib_SOURCE_DIR}>)

target_compile_options(nawarun PUBLIC "-Wl,--export-dynamic")
target_link_options(nawarun PUBLIC "-Wl,--export-dynamic")

if (BuildSharedLib OR BuildTests)
    add_library(nawa SHARED ${NAWA_FILES})
    target_link_libraries(nawa ${NAWA_LINK_LIBRARIES})
    target_include_directories(nawa PUBLIC
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${fastcgilite_BINARY_DIR}/include>
            $<BUILD_INTERFACE:${fastcgilite_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${cppnetlib_SOURCE_DIR}>)
endif ()

if (BuildTests)
    add_executable(unittests
            tests/main.cpp
            tests/unit/encoding_crypto.cpp
            tests/unit/utils.cpp)
    target_link_libraries(unittests nawa Catch2::Catch2)
    target_include_directories(unittests PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

    add_executable(integrationtests
            tests/main.cpp
            tests/integration/environment.cpp)
    target_link_libraries(integrationtests nawa Catch2::Catch2)
    target_include_directories(integrationtests PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

    include(${catch2_SOURCE_DIR}/contrib/Catch.cmake)
    enable_testing()
    catch_discover_tests(unittests)
    catch_discover_tests(integrationtests)

    add_library(nawatest SHARED
            tests/apps/nawatest.cpp)
    target_include_directories(nawatest PUBLIC
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

    add_library(sessiontest SHARED
            tests/apps/sessiontest.cpp)
    target_include_directories(sessiontest PUBLIC
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

    add_library(emailtest SHARED
            tests/apps/emailtest.cpp)
    target_include_directories(emailtest PUBLIC
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

    add_library(gpctest SHARED
            tests/apps/gpctest.cpp)
    target_include_directories(gpctest PUBLIC
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
endif ()

if (BuildExamples)
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

    if (BuildSharedLib)
        add_executable(aslibrary examples/aslibrary.cpp)
        target_link_libraries(aslibrary nawa)
        target_include_directories(aslibrary PUBLIC
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
    endif ()
endif ()

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
if (EXISTS /etc/systemd/system)
    install(FILES "${PROJECT_SOURCE_DIR}/systemd/nawa@.service"
            DESTINATION /etc/systemd/system
            COMPONENT Runtime)
endif ()
if (BuildSharedLib)
    install(TARGETS nawa
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT Runtime)
endif ()

# build documentation
find_package(Doxygen)
if (DOXYGEN_FOUND AND BuildDocs)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in"
            "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" @ONLY)
    add_custom_target(docs
            "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            COMMENT "Generating Doxygen documentation" VERBATIM)
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/docs"
            DESTINATION ${CMAKE_INSTALL_DOCDIR}
            OPTIONAL)
endif ()