#include "../../CryptoFunctions/CryptoExecutor.h"
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

/*
    Server s;
    //s.setUserDirectory("Bruno");
    //s.updateName("/home/roba.txt","/home/roba2.txt");
    //s.remove("/home/sotto/qlc.dar");
    //s.removeDir("/acaso/cart");
    //s.createDir("/home/nuova");
    //s.syncDir("/home/nuova");
*/

    //while(true){
    try {
        boost::asio::io_service ioService;
        Server server;
        ServerSocket serverSocket(ioService, 5000, server);
        ioService.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    //}

}


int main()
{

    //Carica le credenziali in memoria
    const std::string filename("../credenziali.txt");
    loadUsers(filename);

    //Setta la working directory del server
    boost::filesystem::current_path(workingdirectory);

    //launch the pool with num_threads threads
    //Ogni thread gestisce un client (su una porta diversa), tramite una coda di richieste provenienti dallo stesso
    //si possono gestire fino a num_threads clients contemporaneamente
    //boost::asio::thread_pool pool(4); //una coda di thread di dimensione 4: 4 thread alla volta possono essere eseguiti in parallelo

    //submit a function to the pool
    //boost::asio::post(pool,[](){handleSocket(5000);});
    //boost::asio::post(pool,[](){handleSocket(5001);});

    handleSocket(5000);

    //pool.join();

/*
    const std::string filename("../credenziali.txt");
    const std::string username("Bruno");
    const std::string password("passwd3");
    Server s;
    loadUsers(filename);
    s.checkCredenziali(username,password);

    s.checkCredenziali(username);
    std::string user("Bruno");
    std::string passw;
    s.UserToPassword(user,passw);
    std::cout<<passw<<std::endl;
*/

/*
    BIGNUM *prova = BN_new();
    Cookie c;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    Cookie c2(prova);
    std::cout<<c.is_expired()<<std::endl;
*/

    return 0;
}