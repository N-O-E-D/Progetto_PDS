//
// Created by bruno on 25/07/20.
//

#include "Server.h"

using namespace boost::filesystem;

int Server::add(){  //optare per due vesioni add(path,file) per aggiungere file e add(path) per aggiungere solo la directory
    std::cout<<"add()"<<std::endl;
}

int Server::remove(){
    std::cout<<"remove()"<<std::endl;
}

int Server::remove_all(){
    std::cout<<"remove_all()"<<std::endl;
}

int Server::rename() {
    std::cout<<"rename()"<<std::endl;
}

int Server::modify() {
    std::cout<<"modify()"<<std::endl;
}