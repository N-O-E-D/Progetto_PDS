#include "../CryptoFunctions/CryptoExecutor.h"
#include "./API_server/ServerSocket.h"
#include "./Server.h"
#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/filesystem/operations.hpp>  //include boost::filesystem::path.hpp
#include <boost/filesystem/fstream.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/iterator.hpp>
#include <boost/cstdint.hpp>

void handleSocket(int portnum){
    try {
        boost::asio::io_service ioService;
        Server server;
        ServerSocket serverSocket(ioService, 5000, server);
        ioService.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}


int main(int argc, char** argv)
{
    //controllo parametri
    if(argc<2){
        std::cerr<<"Not enough parameters"<<std::endl;
        return -1;
    }
    auto port = std::string(argv[1]);

    //Carica le credenziali in memoria
    const std::string filename("../credenziali.txt");
    if(loadUsers(filename)==INTERNAL_ERROR)
        return -1;

    //Setta la working directory del server
    boost::filesystem::current_path(workingdirectory);

    handleSocket(stoi(port));

    return 0;
}