#include <iostream>

#include <boost/asio/io_service.hpp>

#include "ServerSocket.h"



int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: server <port> <workDirectory>\n";
        return 1;
    }


    try {
        boost::asio::io_service ioService;

        ServerSocket server(ioService, std::stoi(argv[1]), argv[2]);

        ioService.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
