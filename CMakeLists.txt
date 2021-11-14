cmake_minimum_required(VERSION 3.13)
project(nawa)

set(CMAKE_CXX_STANDARD 17)

include(version.cmake)

# build options
option(BuildTests "Build tests" OFF)
option(BuildExamples "Build examples" ON)
option(BuildDocs "Build docs" ON)
option(BuildSharedLib "Build shared library" OFF)
option(BuildStaticLib "Build static library" OFF)
option(BuildNawarun "Build nawarun" ON)
option(EnableArgon2 "Build with argon2 hashing support" ON)

# only unix-based OS are supported
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(NAWA_OS "LINUX")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(NAWA_OS "MACOS")
elseif (UNIX)
    set(NAWA_OS "OTHER_UNIX")
else ()
    message(FATAL_ERROR "Unsupported operating system")
    set(NAWA_OS "UNSUPPORTED")
endif ()

set(NAWA_SOURCE_DIR "${PROJECT_SOURCE_DIR}")

# configure the files that depend on the NAWA version and OS-specific code
include(configure.cmake)

file(GLOB BlowfishCFiles libs/libbcrypt/crypt_blowfish/*.c)
add_library(bcrypt STATIC libs/libbcrypt/bcrypt.c ${BlowfishCFiles})
target_compile_options(bcrypt PUBLIC -fPIC)

# dependencies
include(FetchContent)
FetchContent_Declare(
        fastcgilite
        GIT_REPOSITORY https://github.com/jatofg/fastcgipp.git
        GIT_TAG v3.9.3
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
        include/nawa/AppInit.h
        include/nawa/Exception.h
        include/nawa/RequestHandler/HandleRequestFunction.h
        include/nawa/RequestHandler/RequestHandler.h
        include/nawa/config/Config.h
        include/nawa/connection/Connection.h
        include/nawa/connection/Cookie.h
        include/nawa/filter/AccessFilter/ext/AuthFilter.h
        include/nawa/filter/AccessFilter/ext/BlockFilter.h
        include/nawa/filter/AccessFilter/ext/ForwardFilter.h
        include/nawa/filter/AccessFilter/AccessFilter.h
        include/nawa/filter/AccessFilterList.h
        include/nawa/hashing/HashTypeTable/HashTypeTable.h
        include/nawa/hashing/HashTypeTable/impl/DefaultHashTypeTable.h
        include/nawa/hashing/HashingEngine/HashingEngine.h
        include/nawa/hashing/HashingEngine/impl/BcryptHashingEngine.h
        include/nawa/logging/Log.h
        include/nawa/mail/Email/impl/MimeEmail.h
        include/nawa/mail/Email/impl/SimpleEmail.h
        include/nawa/mail/Email/Email.h
        include/nawa/mail/EmailAddress.h
        include/nawa/mail/SmtpMailer.h
        include/nawa/request/Env.h
        include/nawa/request/File.h
        include/nawa/request/GPC/ext/Post.h
        include/nawa/request/GPC/GPC.h
        include/nawa/request/Request.h
        include/nawa/session/Session.h
        include/nawa/util/crypto.h
        include/nawa/util/encoding.h
        include/nawa/util/MimeMultipart.h
        include/nawa/util/utils.h

        internal/nawa/RequestHandler/impl/FastcgiRequestHandler.h
        internal/nawa/RequestHandler/impl/HttpRequestHandler.h
        internal/nawa/connection/ConnectionInitContainer.h
        internal/nawa/connection/FlushCallbackContainer.h
        internal/nawa/oss.h
        internal/nawa/request/RequestInitContainer.h

        libs/base64/base64.cpp
        libs/base64/base64.h
        libs/inih/ini.c
        libs/inih/ini.h
        libs/punycode/punycode.c
        libs/punycode/punycode.h

        src/AppInit.cpp
        src/RequestHandler/HandleRequestFunction.cpp
        src/RequestHandler/RequestHandler.cpp
        src/RequestHandler/impl/FastcgiRequestHandler.cpp
        src/RequestHandler/impl/HttpRequestHandler.cpp
        src/config/Config.cpp
        src/connection/Connection.cpp
        src/connection/Cookie.cpp
        src/filter/AccessFilter/ext/AuthFilter.cpp
        src/filter/AccessFilter/ext/BlockFilter.cpp
        src/filter/AccessFilter/ext/ForwardFilter.cpp
        src/filter/AccessFilter/AccessFilter.cpp
        src/filter/AccessFilterList.cpp
        src/hashing/HashingEngine/impl/BcryptHashingEngine.cpp
        src/logging/Log.cpp
        src/mail/Email/impl/MimeEmail.cpp
        src/mail/Email/impl/SimpleEmail.cpp
        src/mail/Email/Email.cpp
        src/mail/EmailAddress.cpp
        src/mail/SmtpMailer.cpp
        src/request/Env.cpp
        src/request/File.cpp
        src/request/GPC/ext/Post.cpp
        src/request/GPC/GPC.cpp
        src/request/Request.cpp
        src/session/Session.cpp
        src/util/crypto.cpp
        src/util/encoding.cpp
        src/util/MimeMultipart.cpp
        src/util/utils.cpp
        )

find_package(OpenSSL 1.1.1 REQUIRED)
find_package(Boost 1.66.0 COMPONENTS system thread REQUIRED)
find_package(CURL 7.58 REQUIRED)

set(NAWA_LINK_LIBRARIES
        CURL::libcurl
        OpenSSL::Crypto
        Boost::system
        bcrypt
        fastcgilite
        ${NETLIB_LIBRARIES})

if (EnableArgon2)
    set(NAWA_FILES ${NAWA_FILES}
            include/nawa/hashing/HashingEngine/impl/Argon2HashingEngine.h
            src/hashing/HashingEngine/impl/Argon2HashingEngine.cpp
            src/hashing/HashTypeTable/impl/DefaultHashTypeTable.cpp)
    set(NAWA_LINK_LIBRARIES ${NAWA_LINK_LIBRARIES}
            -largon2)
else ()
    set(NAWA_FILES ${NAWA_FILES}
            src/hashing/HashTypeTable/impl/BcryptOnlyHashTypeTable.cpp)
endif ()

set(NAWA_APP_INCLUDE_DIRS
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
set(NAWA_TEST_INCLUDE_DIRS
        ${NAWA_APP_INCLUDE_DIRS}
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/internal>)
set(NAWA_INSTALL_INCLUDE_DIRS
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>)
set(NAWA_ALL_INCLUDE_DIRS
        ${NAWA_INSTALL_INCLUDE_DIRS}
        ${NAWA_TEST_INCLUDE_DIRS}
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs>
        $<BUILD_INTERFACE:${OPENSSL_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${CURL_INCLUDE_DIRS}>
        $<BUILD_INTERFACE:${fastcgilite_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${fastcgilite_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${cppnetlib_SOURCE_DIR}>)

if (BuildNawarun)
    add_executable(nawarun
            src/nawarun/main.cpp
            internal/nawarun/nawarun.h
            src/nawarun/nawarun.cpp
            ${NAWA_FILES})

    target_link_libraries(nawarun
            -ldl
            ${NAWA_LINK_LIBRARIES})
    target_include_directories(nawarun PUBLIC ${NAWA_ALL_INCLUDE_DIRS})

    target_compile_options(nawarun PUBLIC "-Wl,--export-dynamic")
    if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        target_link_options(nawarun PUBLIC "-Wl,-export_dynamic")
    else ()
        target_link_options(nawarun PUBLIC "-Wl,--export-dynamic")
    endif ()
endif ()

if (BuildSharedLib OR BuildTests)
    add_library(nawa SHARED ${NAWA_FILES})
    target_link_libraries(nawa ${NAWA_LINK_LIBRARIES})
    target_include_directories(nawa PUBLIC ${NAWA_ALL_INCLUDE_DIRS})
endif ()

if (BuildStaticLib)
    add_library(nawa_static STATIC ${NAWA_FILES})
    target_link_libraries(nawa_static ${NAWA_LINK_LIBRARIES})
    target_include_directories(nawa_static PUBLIC ${NAWA_ALL_INCLUDE_DIRS})
endif ()

if (BuildTests)
    set(NAWA_ENCODING_CRYPTO_UNITTEST "")
    if (EnableArgon2)
        set(NAWA_ENCODING_CRYPTO_UNITTEST
                tests/unit/encoding_crypto.cpp)
    endif ()
    add_executable(unittests
            tests/main.cpp
            tests/unit/email.cpp
            tests/unit/sessions.cpp
            tests/unit/utils.cpp
            ${NAWA_ENCODING_CRYPTO_UNITTEST})
    target_link_libraries(unittests nawa Catch2::Catch2)
    target_include_directories(unittests PUBLIC
            ${NAWA_TEST_INCLUDE_DIRS})

    add_executable(integrationtests
            tests/main.cpp
            tests/integration/http.cpp)
    target_link_libraries(integrationtests nawa Catch2::Catch2)
    target_include_directories(integrationtests PUBLIC
            ${NAWA_TEST_INCLUDE_DIRS})

    include(${catch2_SOURCE_DIR}/contrib/Catch.cmake)
    enable_testing()
    catch_discover_tests(unittests)
    catch_discover_tests(integrationtests)

    if (BuildNawarun)
        add_library(nawatest SHARED
                tests/apps/nawatest.cpp)
        target_include_directories(nawatest PUBLIC
                ${NAWA_INSTALL_INCLUDE_DIRS}
                ${NAWA_APP_INCLUDE_DIRS})

        add_library(sessiontest SHARED
                tests/apps/sessiontest.cpp)
        target_include_directories(sessiontest PUBLIC
                ${NAWA_INSTALL_INCLUDE_DIRS}
                ${NAWA_APP_INCLUDE_DIRS})

        add_library(emailtest SHARED
                tests/apps/emailtest.cpp)
        target_include_directories(emailtest PUBLIC
                ${NAWA_INSTALL_INCLUDE_DIRS}
                ${NAWA_APP_INCLUDE_DIRS})

        add_library(gpctest SHARED
                tests/apps/gpctest.cpp)
        target_include_directories(gpctest PUBLIC
                ${NAWA_INSTALL_INCLUDE_DIRS}
                ${NAWA_APP_INCLUDE_DIRS})

        if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            set_target_properties(nawatest PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
            set_target_properties(sessiontest PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
            set_target_properties(emailtest PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
            set_target_properties(gpctest PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
        endif ()
    endif ()
endif ()

if (BuildExamples)
    if (BuildNawarun)
        add_library(helloworld SHARED
                examples/helloworld.cpp)
        target_include_directories(helloworld PUBLIC
                ${NAWA_INSTALL_INCLUDE_DIRS}
                ${NAWA_APP_INCLUDE_DIRS})

        add_library(contactform SHARED
                examples/contactform.cpp)
        target_include_directories(contactform PUBLIC
                ${NAWA_INSTALL_INCLUDE_DIRS}
                ${NAWA_APP_INCLUDE_DIRS})

        add_library(multipage SHARED
                examples/multipage.cpp)
        target_include_directories(multipage PUBLIC
                ${NAWA_INSTALL_INCLUDE_DIRS}
                ${NAWA_APP_INCLUDE_DIRS})

        if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            set_target_properties(helloworld PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
            set_target_properties(contactform PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
            set_target_properties(multipage PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
        endif ()
    endif ()

    if (BuildSharedLib)
        add_executable(aslibrary examples/aslibrary.cpp)
        target_link_libraries(aslibrary nawa)
        target_include_directories(aslibrary PUBLIC
                ${NAWA_INSTALL_INCLUDE_DIRS}
                ${NAWA_APP_INCLUDE_DIRS})
    endif ()

    if (BuildStaticLib)
        add_executable(aslibrary_static examples/aslibrary.cpp)
        target_link_libraries(aslibrary_static nawa_static)
        target_include_directories(aslibrary PUBLIC
                ${NAWA_INSTALL_INCLUDE_DIRS}
                ${NAWA_APP_INCLUDE_DIRS})
    endif ()
endif ()

# install targets, components should be checked again w.r.t. packaging
include(GNUInstallDirs)
if (BuildNawarun)
    install(TARGETS nawarun
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT Runtime)
    install(FILES "${PROJECT_SOURCE_DIR}/config.ini"
            DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/nawa
            COMPONENT Runtime)
endif ()
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
if (BuildDocs)
    add_subdirectory(docs)
endif()