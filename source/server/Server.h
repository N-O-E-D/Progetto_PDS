//
// Created by bruno on 25/07/20.
//

#ifndef PROVA_SOCKET_ASIO_SERVER_H
#define PROVA_SOCKET_ASIO_SERVER_H

#include "../../HashExecutor/HashExecutor.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/system/error_code.hpp>
#include <boost/filesystem/operations.hpp>  //include boost::filesystem::path.hpp
#include <boost/filesystem/fstream.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/iterator.hpp>
#include <boost/cstdint.hpp>

enum responseType{
    OK, NOT_PRESENT, OLD_VERSION, INTERNAL_ERROR
};

class Server {

public:
    responseType update(std::string const& path, const std::vector<char>& recbuffer, const ssize_t& buffsize);
    responseType updateName(std::string const& path, std::string const& newName);
    responseType remove(std::string const& path);
    responseType removeDir(std::string const& path);
    responseType createFile(std::string const& path, const std::vector<char>& recbuffer, const ssize_t& buffsize);
    responseType createDir(std::string const& path);
    responseType syncDir(std::string const& path);
    responseType syncFile(std::string const& path,unsigned char* md_value,unsigned int md_len);
};


#endif //PROVA_SOCKET_ASIO_SERVER_H
