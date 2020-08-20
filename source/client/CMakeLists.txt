cmake_minimum_required(VERSION 3.16)
project(client)

set(CMAKE_CXX_STANDARD 17)

add_executable(client main.cpp ../API_client/ClientSocket.cpp ../API_client/ClientSocket.h FileSystemWatcher.h FileSystemWatcher.cpp PathStatusMap.h)

# OpenSSL Libraries
#set(OPENSSL_ROOT_DIR /usr/local/Cellar/openssl@1.1/1.1.1g) #for OpenSSL installed with brew on macOS
find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    target_link_libraries(client ${OPENSSL_LIBRARIES})
else()
    message(FATAL_ERROR "Could not find OpenSSL")
endif()

# Boost Libraries
find_package(Boost COMPONENTS log log_setup thread filesystem system REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(client
            ${Boost_LOG_SETUP_LIBRARY}
            ${Boost_LOG_LIBRARY}
            ${Boost_FILESYSTEM_LIBRARY}
            ${Boost_THREAD_LIBRARY}
            ${Boost_SYSTEM_LIBRARY}
            )
else()
    message(FATAL_ERROR "Could not find Boost")
endif()


