
#include "../../CryptoFunctions/CryptoExecutor.h"
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
#include "./Server.h"
#include "../API_server/ServerSocket.h"

//create a Server object (provides ADD, MODIFY, REMOVE methods)
Server server;

std::vector<char> bufferprova = {'c','i','a','o'};
ssize_t buffsize = bufferprova.size();

std::map<std::string,int> commands = {
        {"UPDATE",1},
        {"UPDATENAME",2},
        {"REMOVE",3},
        {"REMOVEDIR",4},
        {"CREATEFILE",5},
        {"CREATEDIR",6},
        {"SYNCDIR",7},
        {"SYNCFILE",8},
};

using boost::asio::ip::tcp;

int selectComando(std::string& str, std::string& path, std::string& newpath){
    str.pop_back();
    str.pop_back();
    auto cmd = str.substr(0,str.find(" "));  //i comandi sono del tipo "UPDATE path", quindi prendo la sottostringa fino al primo spazio, quindi il comando
    str.erase(0,cmd.length()+1);
    path = str.substr(0,str.find(" "));
    str.erase(0,path.length()+1);
    newpath = str;
    std::cout<<cmd<<" "<<path<<" "<<newpath<<std::endl;
    return commands[cmd];
}
/*
bool autentica(boost::asio::streambuf& recmessage){
    //from streambuf to string
    std::string recmex(boost::asio::buffers_begin(recmessage.data()),boost::asio::buffers_begin(recmessage.data())+recmessage.size());
    auto username = recmex.substr(0,recmex.find(" "));
    auto password = recmex.substr(username.length()+1);
    std::cout<<username<<std::endl;
    password.pop_back();  //rimuove "\n" (next line)
    password.pop_back();  //rimuove "\r" (carriage return)
    std::cout<<password<<std::endl;
    std::cout<<users.at(username)<<std::endl;
    try{
        if(password.compare(users.at(username))==0) {
            std::cout<<"Benvenuto, "<<username<<"!"<<std::endl;
            return true;
        }
        else {
            std::cout<<"Password non corretta."<<std::endl;
            return false;
        }
    }
    catch(std::out_of_range& exception){
        std::cout<<"eccezione"<<std::endl;
        return false;}

}
*/

void handleSocket(int portnum){

    Server s;
    s.createDir("prova.giar");
    s.setUserDirectory("bruno");
    /*
    while(true){
    try {
        boost::asio::io_service ioService;
        Server server;
        ServerSocket serverSocket(ioService, 5000, server);

        ioService.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    }
    */

    /*
    try
    {
        // Any program that uses asio need to have at least one io_service object
        boost::asio::io_service io_service;
        //std::cout<<"ioservice created"<<std::endl;

        // acceptor object needs to be created to listen for new connections
        tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), portnum));
        //std::cout<<"acceptor created"<<std::endl;

        while (true)
        {
            // creates a socket
            tcp::socket socket(io_service);
            //std::cout<<"socket created"<<std::endl;

            // wait and listen
            std::cout<<"Waiting for connection..."<<std::endl;
            acceptor.accept(socket);

            // prepare message to send back to client
            auto sendmessage = boost::asio::buffer("Hello from server!\nPlease authenticate (username password):");

            //boost::system::error_code ignored_error;

            // writing the message
            //socket.write_some(sendmessage, ignored_error);
            boost::asio::write(socket,boost::asio::buffer(sendmessage));



            //prepare buf to store received message by client
            boost::asio::streambuf recmessage;

            //receive something on the socket
            //int size = socket.read_some(recmessage,ignored_error);
            //socket.write_some(recmessage, ignored_error);
            boost::asio::read_until(socket,recmessage,"\n");

            if(autentica(recmessage)) {
                recmessage.consume(recmessage.size());  //pulisce il buffer in ricezione...
                boost::asio::read_until(socket,recmessage,"\n");  //...per riempirlo subito dopo

                //from streambuf to string
                std::string recmex(boost::asio::buffers_begin(recmessage.data()),boost::asio::buffers_begin(recmessage.data())+recmessage.size());
                std::string path;
                std::string newpath;

                //hash di un file a caso
                unsigned char md_value[EVP_MAX_MD_SIZE];
                unsigned int md_len = computeHash("../CMakeLists.txt", md_value);

                switch (selectComando(recmex,path,newpath)) {
                    case 1:
                        server.update(path, bufferprova, buffsize);  //ok
                        break;
                    case 2:
                        server.updateName(path,newpath);  //ok
                        break;
                    case 3:
                        server.remove(path);  //ok
                        break;
                    case 4:
                        server.removeDir(path);  //ok
                        break;
                    case 5:
                        server.createFile(path, bufferprova, buffsize);  //ok
                        break;
                    case 6:
                        server.createDir(path); //ok
                        break;
                    case 7:
                        server.syncDir(path);  //ok
                        break;
                    case 8:
                        server.syncFile(path,md_value,md_len);
                        break;
                    default:
                        std::cout << "Comando errato" << std::endl;
                        break;
                }

            }

            //boost::asio::write(socket,recmessage);
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return;
     */
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
    boost::asio::thread_pool pool(4); //una coda di thread di dimensione 4: 4 thread alla volta possono essere eseguiti in parallelo

    //submit a function to the pool
    boost::asio::post(pool,[](){handleSocket(5000);});
    //boost::asio::post(pool,[](){handleSocket(5001);});

    pool.join();
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

    return 0;
*/
}