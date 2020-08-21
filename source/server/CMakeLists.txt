cmake_minimum_required(VERSION 3.16)
project(server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -pthread)

add_executable(server main.cpp Server.cpp Server.h ../../CryptoFunctions/CryptoExecutor.cpp ../../CryptoFunctions/CryptoExecutor.h ../API_server/ServerSocket.cpp ../API_server/ServerSocket.h)

# OpenSSL Libraries
#set(OPENSSL_ROOT_DIR /usr/local/Cellar/openssl@1.1/1.1.1g) #for OpenSSL installed with brew on macOS
find_package(OpenSSL)
if(OpenSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    target_link_libraries(server ${OPENSSL_LIBRARIES})
else()
    message(FATAL_ERROR "Could not find OpenSSL")
endif()

# Boost Libraries
find_package(Boost COMPONENTS system thread filesystem log log_setup REQUIRED)
set (CMAKE_CXX_FLAGS "-g -Wall -DBOOST_LOG_DYN_LINK") #per linux Lorenzo
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(server ${Boost_LIBRARIES})
else()
    message(FATAL_ERROR "Could not find Boost")
endif()






