cmake_minimum_required(VERSION 3.16)
project(API_client)

set(CMAKE_CXX_STANDARD 17)

add_executable(API_client ClientSocket.cpp ClientSocket.h main.cpp )

# OpenSSL Libraries
find_package(OpenSSL)
if(OpenSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    target_link_libraries(API_client ${OPENSSL_LIBRARIES})
else()
    message(FATAL_ERROR "Could not find OpenSSL")
endif()

# Boost Libraries
find_package(Boost COMPONENTS thread system filesystem log log_setup REQUIRED)
set (CMAKE_CXX_FLAGS "-g -Wall -DBOOST_LOG_DYN_LINK")
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(API_client ${Boost_LIBRARIES})
else()
    message(FATAL_ERROR "Could not find Boost")
endif()




