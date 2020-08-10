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
#include "Server.h"

//create a Server object (provides ADD, MODIFY, REMOVE methods)
Server server;

std::map<std::string,int> commands = {
        {"ADD",1},
        {"REMOVE",2},
        {"REMOVE_ALL",3},
        {"RENAME",4},
        {"MODIFY",5}
};

std::map<std::string,std::string> users = {
        {"Lorenzo","passwd1"},
        {"Giandonato","passwd2"},
        {"Bruno","passwd3"}
};

using boost::asio::ip::tcp;

int selectComando(const std::string& str){
    auto cmd = str.substr(0,str.find(" "));  //i comandi sono del tipo "UPDATE path", quindi prendo la sottostringa fino al primo spazio, quindi il comando
    return commands[cmd];
}

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

void handleSocket(int portnum){
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

                switch (selectComando(recmex)) {
                    case 1:
                        server.add();
                        break;
                    case 2:
                        server.remove();
                        break;
                    case 3:
                        server.remove_all();
                        break;
                    case 4:
                        server.rename();
                        break;
                    case 5:
                        server.modify();
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
}



int main()
{

    //launch the pool with num_threads threads
    //Ogni thread gestisce un client (su una porta diversa), tramite una coda di richieste provenienti dallo stesso
    //si possono gestire fino a num_threads clients contemporaneamente
    boost::asio::thread_pool pool(4); //una coda di thread di dimensione 4: 4 thread alla volta possono essere eseguiti in parallelo

    //submit a function to the pool
    boost::asio::post(pool,[](){handleSocket(5000);});
    //boost::asio::post(pool,[](){handleSocket(5001);});

    pool.join();

    return 0;
}