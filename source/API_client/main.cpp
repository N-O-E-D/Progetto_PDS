#include <iostream>

#include <boost/asio/io_service.hpp>

#include "ClientSocket.h"


int main(int argc, char* argv[])
{
    if (argc != 4) {
        std::cerr << "Usage: client <address> <port> <filePath>\n";
        return 1;
    }


    auto address = argv[1];
    auto port = argv[2];
    auto filePath = argv[3];

    try {
        boost::asio::io_service ioService;

        boost::asio::ip::tcp::resolver resolver(ioService);
        auto endpointIterator = resolver.resolve({ address, port });
        ClientSocket client(ioService, endpointIterator);
        //unsigned char md_value[EVP_MAX_MD_SIZE];
        //unsigned int md_len=computeHash(filePath,md_value);
        //client.createFile(filePath);
        client.authenticate("Lorenzo","passwd1");
        ioService.run();

    } catch (std::fstream::failure& e) {
        std::cerr << e.what() << "\n";
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
