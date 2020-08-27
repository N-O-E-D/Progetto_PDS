cmake_minimum_required(VERSION 3.16)
project(client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -pthread)

add_executable(client main.cpp ../API_client/ClientSocket.cpp ../API_client/ClientSocket.h FileSystemWatcher.h FileSystemWatcher.cpp PathStatusMap.h ../../CryptoFunctions/CryptoExecutor.cpp ../../CryptoFunctions/CryptoExecutor.h)

# OpenSSL Libraries
if (APPLE)
    # This is a bug in CMake that causes it to prefer the system version over
    # the one in the specified ROOT folder.
    set(OPENSSL_ROOT_DIR ${OPENSSL_ROOT_DIR} /usr/local/Cellar/openssl@1.1/1.1.1g/)
    set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libcrypto.dylib CACHE FILEPATH "" FORCE)
    set(OPENSSL_SSL_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libssl.dylib CACHE FILEPATH "" FORCE)
endif()
find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    target_link_libraries(client ${OPENSSL_LIBRARIES})
else()
    message(FATAL_ERROR "Could not find OpenSSL")
endif()

# Boost Libraries
find_package(Boost COMPONENTS log log_setup thread filesystem system REQUIRED)
set (CMAKE_CXX_FLAGS "-g -Wall -DBOOST_LOG_DYN_LINK") #per linux Lorenzo
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(client ${Boost_LIBRARIES})
else()
    message(FATAL_ERROR "Could not find Boost")
endif()


