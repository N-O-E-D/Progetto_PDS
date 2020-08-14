//
// Created by bruno on 25/07/20.
//

#include "Server.h"

using namespace boost::filesystem;

bool Server::update(std::string const& path, const std::vector<char>& recbuffer, const ssize_t& buffsize){

    std::cout<<"UPDATE"<<std::endl;

    //crea un file con nome uguale (ma con minima differenza per non rimpiazzare l'originale)
    std::string tmpname(path+"tmp");
    std::ofstream recfile(tmpname, std::ofstream::binary);

    if(recfile.is_open()){
        try{
            recfile.write(recbuffer.data(),buffsize);
        }
        catch(std::exception& e){  //in caso di errore
            std::cerr << "Write tmp file error: " << e.what() << std::endl;
            remove(tmpname);  //rimuovi il file provvisorio
            return false;
        }
        std::cout<<"Write tmp file success!"<<std::endl;
        recfile.close();
        //in caso di successo rimuovi il file vecchio e rinomina il file temporaneo
        if(remove(path)){
            updateName(tmpname,path);
        }
        return true;
    }
    std::cout<<"Error opening file "<<path<<std::endl;
    return false;

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


bool Server::createFile(std::string const& path, const std::vector<char>& recbuffer, const ssize_t& buffsize){

    std::cout<<"CREATEFILE"<<std::endl;

    std::ofstream recfile(path, std::ofstream::binary);

    if(recfile.is_open()){
        try{
            recfile.write(recbuffer.data(),buffsize);
        }
        catch(std::exception& e){  //in caso di errore
            std::cerr << "Write file error: " << e.what() << std::endl;
            remove(path);  //rimuovi il file provvisorio
            return false;
        }
        std::cout<<"Write file success!"<<std::endl;
        recfile.close();
        return true;
    }
    std::cout<<"Error opening file "<<path<<std::endl;
    return false;
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