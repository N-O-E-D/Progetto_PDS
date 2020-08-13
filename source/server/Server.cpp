//
// Created by bruno on 25/07/20.
//

#include "Server.h"

using namespace boost::filesystem;

bool Server::update(std::string const& path){

    std::cout<<"UPDATE"<<std::endl;

}
bool Server::updateName(std::string const& path, std::string const& newName){

    const boost::filesystem::path oldp(path);
    const boost::filesystem::path newp(newName);

    std::cout<<"UPDATENAME"<<std::endl;

    /*boost::system::error_code ec;
    try{
        rename(oldp, newp, ec);
        if(ec){
            std::cerr<<ec.message()<<std::endl;
        }
    }
    catch(std::exception& e){
        std::cerr << e.what() << std::endl;
    }*/

}
bool Server::remove(std::string const& path){

    std::cout<<"REMOVE"<<std::endl;

}
bool Server::removeDir(std::string const& path){

    std::cout<<"REMOVEDIR"<<std::endl;

}
bool Server::createFile(std::string const& path){

    std::cout<<"CREATEFILE"<<std::endl;

}
bool Server::createDir(std::string const& path){

    std::cout<<"CREATEDIR"<<std::endl;

}
bool Server::syncDir(std::string const& path){

    std::cout<<"SYNCDIR"<<std::endl;

}
/*bool Server::syncFile(std::string const& path,unsigned char* md_value,unsigned int md_len){

    std::cout<<"SYNCFILE"<<std::endl;

}*/
bool Server::syncFile(std::string const& path){

    std::cout<<"SYNCFILE"<<std::endl;

}