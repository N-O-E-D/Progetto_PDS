//
// Created by bruno on 25/07/20.
//

#ifndef PROVA_SOCKET_ASIO_SERVER_H
#define PROVA_SOCKET_ASIO_SERVER_H

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/filesystem/operations.hpp>  //include boost::filesystem::path.hpp
#include <boost/filesystem/fstream.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/iterator.hpp>
#include <boost/cstdint.hpp>

class Server {

public:
    int add();
    int remove();
    int remove_all();
    int rename();
    int modify();
};


#endif //PROVA_SOCKET_ASIO_SERVER_H
