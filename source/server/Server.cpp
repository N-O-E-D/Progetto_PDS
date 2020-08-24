//
// Created by bruno on 25/07/20.
//

#include "Server.h"

//using namespace boost::filesystem;



/* ***** FILESYSTEM ***** */

responseType Server::update(std::string const& path, const std::vector<char>& recbuffer, const ssize_t& buffsize){

    std::cout<<"UPDATE"<<std::endl;

    const boost::filesystem::path p(path);
    if(!boost::filesystem::exists(p)){
        std::cout<<"Il path "<<p<<" non esiste!"<<std::endl;
        return NOT_PRESENT;
    }

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
            return INTERNAL_ERROR;
        }
        std::cout<<"Write tmp file success!"<<std::endl;
        recfile.close();
        //in caso di successo rimuovi il file vecchio e rinomina il file temporaneo
        if(remove(path)==OK){
            updateName(tmpname,path);
            return OK;
        }
        std::cout<<"Error removing tmp file"<<std::endl;
        return INTERNAL_ERROR;
    }
    std::cout<<"Error opening file "<<path<<std::endl;
    return INTERNAL_ERROR;

}

responseType Server::updateName(std::string const& path, std::string const& newName){  //riceve in ingresso il vecchio path e il nuovo path

    std::cout<<"UPDATENAME"<<std::endl;

    const boost::filesystem::path oldp(path);
    const boost::filesystem::path newp(newName);

    if(!boost::filesystem::exists(oldp)){
        std::cout<<"Il path "<<oldp<<" non esiste!"<<std::endl;
        return NOT_PRESENT;
    }

    boost::system::error_code ec;
    rename(oldp, newp, ec);  //questa versione di rename prende il codice errore, pertanto non c'è bisogno di inserirla in un try/catch
    if(ec){   //basta verificare la presenza dell'errore o meno
        std::cerr<<"Update name error: "<<ec.message()<<std::endl;
        return INTERNAL_ERROR;
    }
    std::cout<<"Update name success!"<<std::endl;
    return OK;
}

responseType Server::remove(std::string const& path){  //se path è una directory, elimina solo se la directory è vuota

    std::cout<<"REMOVE"<<std::endl;

    const boost::filesystem::path p(path);

    if(!boost::filesystem::exists(p)){
        std::cout<<"Il path "<<p<<" non esiste!"<<std::endl;
        return NOT_PRESENT;
    }

    boost::system::error_code ec;
    boost::filesystem::remove(p,ec);  //questa versione di remove prende il codice errore, pertanto non c'è bisogno di inserirla in un try/catch
    if(ec){   //basta verificare la presenza dell'errore o meno
        std::cerr<<"Remove error: "<<ec.message()<<std::endl;
        return INTERNAL_ERROR;
    }
    std::cout<<"Remove success!"<<std::endl;
    return OK;
}

responseType Server::removeDir(std::string const& path){  //se path è un file, rimuove solo il file

    std::cout<<"REMOVEDIR"<<std::endl;

    const boost::filesystem::path p(path);

    if(!boost::filesystem::exists(p)){
        std::cout<<"Il path "<<p<<" non esiste!"<<std::endl;
        return NOT_PRESENT;
    }

    boost::system::error_code ec;
    boost::filesystem::remove_all(p,ec);  //questa versione di remove_all prende il codice errore, pertanto non c'è bisogno di inserirla in un try/catch
    if(ec){   //basta verificare la presenza dell'errore o meno
        std::cerr<<"Remove dir error: "<<ec.message()<<std::endl;
        return INTERNAL_ERROR;
    }
    std::cout<<"Remove dir success!"<<std::endl;
    return OK;
}


responseType Server::createFile(std::string const& path, const std::vector<char>& recbuffer, const ssize_t& buffsize){

    std::cout<<"CREATEFILE"<<std::endl;

    std::ofstream recfile(path, std::ofstream::binary);

    if(recfile.is_open()){
        try{
            recfile.write(recbuffer.data(),buffsize);
        }
        catch(std::exception& e){  //in caso di errore
            std::cerr << "Write file error: " << e.what() << std::endl;
            remove(path);  //rimuovi il file provvisorio
            return INTERNAL_ERROR;
        }
        std::cout<<"Write file success!"<<std::endl;
        recfile.close();
        return OK;
    }
    std::cout<<"Error opening file "<<path<<std::endl;
    return INTERNAL_ERROR;
}

responseType Server::createDir(std::string const& path){

    std::cout<<"CREATEDIR"<<std::endl;

    const boost::filesystem::path p(path);

    boost::system::error_code ec;
    boost::filesystem::create_directory(p,ec);  //questa versione di create_directory prende il codice errore, pertanto non c'è bisogno di inserirla in un try/catch
    if(ec){   //basta verificare la presenza dell'errore o meno
        std::cerr<<"Create dir error: "<<ec.message()<<std::endl;
        return INTERNAL_ERROR;
    }
    std::cout<<"Create dir success!"<<std::endl;
    return OK;

}

responseType Server::syncDir(std::string const& path){

    std::cout<<"SYNCDIR"<<std::endl;

    const boost::filesystem::path p(path);

    if(boost::filesystem::exists(p)) {
        std::cout<<"Il path esiste!"<<std::endl;
        return OK;
    }
    std::cout<<"Il path non esiste!"<<std::endl;
    return NOT_PRESENT;
}

responseType Server::syncFile(std::string const& path, unsigned char* md_value, unsigned int md_len) {

    std::cout << "SYNCFILE" << std::endl;

    const boost::filesystem::path p(path);

    //hash di <path>
    unsigned char md_value_path[EVP_MAX_MD_SIZE];
    unsigned int md_len_path = computeHash(path, md_value_path);

    if (boost::filesystem::exists(p)) {
        if (compareHash(md_value, md_value_path, md_len)) {
            std::cout << "Hash uguali" << std::endl;
            return OK;
        }
        std::cout << "Hash non uguali" << std::endl;
        return OLD_VERSION;
    }
    std::cout<<"Il path non esiste"<<std::endl;
    return NOT_PRESENT;
}



/* ***** AUTENTICAZIONE ***** */

static std::map<std::string,std::string> credenziali;

responseType loadUsers(const std::string& filename){ //default filename: "../credenziali.txt"
    std::ifstream file;

    std::string key;
    std::string value;
    file.open(filename);

    if(file.is_open()){
        while(file >> key >> value){
            credenziali[key]=value;
        }
        std::cout<<"Credenziali caricate correttamente!"<<std::endl;
        return OK;
    }
    else{
        std::cout<<"Errore apertura file "<<filename<<std::endl;
        return INTERNAL_ERROR;
    }
}

responseType Server::checkCredenziali(const std::string& username, const std::string& password){

    if(credenziali.find(username) != credenziali.end()){  //se esiste la chiave username
        if(password.compare(credenziali.at(username))==0){
            std::cout<<"Credenziali corrette. Benvenuto "<<username<<"!"<<std::endl;
            return OK;
        }
        else{
            std::cout<<"Password errata, "<<username<<"!"<<std::endl;
            return WRONG_PASSWORD;
        }
    }
    else{
        std::cout<<"Username non esistente"<<std::endl;
        return WRONG_USERNAME;
    }

}

responseType Server::checkCredenziali(const std::string& username){

    if(credenziali.find(username) != credenziali.end()){  //se esiste la chiave username
        std::cout<<"Username corretto"<<std::endl;
        return OK;
    }
    else{
        std::cout<<"Username non esistente"<<std::endl;
        return WRONG_USERNAME;
    }
}

responseType Server::UserToPassword(const std::string& username, std::string& password){
    if(credenziali.find(username) != credenziali.end()) {  //se esiste la chiave username
        password=credenziali.at(username);
        return OK;
    }
    else{
        std::cout<<"Username non esistente"<<std::endl;
        return WRONG_USERNAME;
    }
}