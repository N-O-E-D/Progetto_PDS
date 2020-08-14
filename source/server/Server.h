//
// Created by bruno on 25/07/20.
//

#ifndef PROVA_SOCKET_ASIO_SERVER_H
#define PROVA_SOCKET_ASIO_SERVER_H

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



class Server {

public:
    bool update(std::string const& path, const std::vector<char>& recbuffer, const ssize_t& buffsize);
    bool updateName(std::string const& path, std::string const& newName);
    bool remove(std::string const& path);
    bool removeDir(std::string const& path);
    bool createFile(std::string const& path, const std::vector<char>& recbuffer, const ssize_t& buffsize);
    bool createDir(std::string const& path);
    bool syncDir(std::string const& path);
    //bool syncFile(std::string const& path,unsigned char* md_value,unsigned int md_len);
    bool syncFile(std::string const& path);
};


#endif //PROVA_SOCKET_ASIO_SERVER_H
