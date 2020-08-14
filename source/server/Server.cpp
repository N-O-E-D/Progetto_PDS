//
// Created by bruno on 25/07/20.
//

#include "Server.h"

using namespace boost::filesystem;

bool Server::update(std::string const& path){

    std::cout<<"UPDATE"<<std::endl;

}

bool Server::updateName(std::string const& path, std::string const& newName){  //riceve in ingresso il vecchio path e il nuovo path

    std::cout<<"UPDATENAME"<<std::endl;

    const boost::filesystem::path oldp(path);
    const boost::filesystem::path newp(newName);

    boost::system::error_code ec;
    rename(oldp, newp, ec);  //questa versione di rename prende il codice errore, pertanto non c'è bisogno di inserirla in un try/catch
    if(ec){   //basta verificare la presenza dell'errore o meno
        std::cerr<<"Update name error: "<<ec.message()<<std::endl;
        return false;
    }
    std::cout<<"Update name success!"<<std::endl;
    return true;
}

bool Server::remove(std::string const& path){  //se path è una directory, elimina solo se la directory è vuota

    std::cout<<"REMOVE"<<std::endl;

    const boost::filesystem::path p(path);

    boost::system::error_code ec;
    boost::filesystem::remove(p,ec);  //questa versione di remove prende il codice errore, pertanto non c'è bisogno di inserirla in un try/catch
    if(ec){   //basta verificare la presenza dell'errore o meno
        std::cerr<<"Remove error: "<<ec.message()<<std::endl;
        return false;
    }
    std::cout<<"Remove success!"<<std::endl;
    return true;
}

bool Server::removeDir(std::string const& path){  //se path è un file, rimuove solo il file

    std::cout<<"REMOVEDIR"<<std::endl;

    const boost::filesystem::path p(path);

    boost::system::error_code ec;
    boost::filesystem::remove_all(p,ec);  //questa versione di remove_all prende il codice errore, pertanto non c'è bisogno di inserirla in un try/catch
    if(ec){   //basta verificare la presenza dell'errore o meno
        std::cerr<<"Remove dir error: "<<ec.message()<<std::endl;
        return false;
    }
    std::cout<<"Remove dir success!"<<std::endl;
    return true;
}



bool Server::createFile(std::string const& path){

    std::cout<<"CREATEFILE"<<std::endl;

}

bool Server::createDir(std::string const& path){

    std::cout<<"CREATEDIR"<<std::endl;

    const boost::filesystem::path p(path);

    boost::system::error_code ec;
    boost::filesystem::create_directory(p,ec);  //questa versione di create_directory prende il codice errore, pertanto non c'è bisogno di inserirla in un try/catch
    if(ec){   //basta verificare la presenza dell'errore o meno
        std::cerr<<"Create dir error: "<<ec.message()<<std::endl;
        return false;
    }
    std::cout<<"Create dir success!"<<std::endl;
    return true;

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