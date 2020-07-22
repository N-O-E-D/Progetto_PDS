cmake_minimum_required(VERSION 3.16)
project(client)

set(CMAKE_CXX_STANDARD 17)

add_executable(client main.cpp)

# OpenSSL Libraries
# set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
find_package(OpenSSL)

if(OpenSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    target_link_libraries(client ${OPENSSL_LIBRARIES})
else()
    message(FATAL_ERROR "Could not find OpenSSL")
endif()

# Boost Libraries
find_package(Boost)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(client ${Boost_LIBRARIES})
else()
    message(FATAL_ERROR "Could not find Boost")
endif()




