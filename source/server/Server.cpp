//
// Created by bruno on 25/07/20.
//

#include "Server.h"

//using namespace boost::filesystem;

//
// Created by Giandonato Farina on 29/08/2020.
//
std::string pathManipulation(const std::string& path){
    using namespace std;
    string newPath;
    // check if start with ../
    auto pos = path.find("../");
    if( pos != string::npos ) {
        newPath = path.substr(pos + 3);
        return newPath;
    }
    // check if start with ./
    pos = path.find("./");
    if( pos != string::npos ) {
        newPath = path.substr(pos + 2);
        return newPath;
    }
    return path;
}

/* ***** FILESYSTEM ***** */

void Server::setUserDirectory(const std::string& username){  //se la directory dell'utente non esiste, la crea

    const std::string ud(workingdirectory+"/"+username);
    const boost::filesystem::path p(ud);
    if(!boost::filesystem::exists(p)){
        boost::system::error_code ec;
        boost::filesystem::create_directory(p,ec);  //questa versione di create_directory prende il codice errore, pertanto non c'è bisogno di inserirla in un try/catch
        if(ec){   //basta verificare la presenza dell'errore o meno
            std::cerr<<"Create dir error: "<<ec.message()<<std::endl;
            return;
        }
        std::cout<<"User directory non esistente: "<<p<<" creato."<<std::endl;
    }
    userDirectory = p;
}

responseType Server::update(std::string const& path, const std::vector<char>& recbuffer, const ssize_t& buffsize){

    std::cout<<"UPDATE"<<std::endl;

    boost::system::error_code ec;
    auto userPath = userDirectory / pathManipulation(path);

    if(!boost::filesystem::exists(userPath)){
        std::cout<<"Il path "<<userPath<<" non esiste!"<<std::endl;
        return NOT_PRESENT;
    }

    //crea un file con nome uguale (ma con minima differenza per non rimpiazzare l'originale)
    std::string tmpname(userPath.string()+"tmp");
    std::ofstream recfile;
    recfile.exceptions(std::ios_base::failbit | std::ios_base::badbit);  //tells the file to throw an exeption if those bit are set

    recfile.open(tmpname, std::ofstream::binary | std::ofstream::app);

    if(recfile.is_open()){
        try{
            recfile.write(recbuffer.data(),buffsize);
        }
        catch(std::exception& e){  //in caso di errore
            std::cerr << "Write tmp file error: " << e.what() << std::endl;
            boost::filesystem::remove(tmpname,ec);  //rimuovi il file provvisorio
            if(!ec)
                return OK;
            std::cout<<"Error removing tmp file"<<std::endl;
            return INTERNAL_ERROR;
        }
        std::cout<<"Write tmp file success!"<<std::endl;
        recfile.close();

        //in caso di successo rimuovi il file vecchio e rinomina il file temporaneo
        boost::filesystem::remove(userPath,ec);
        if(!ec){
            //updateName(tmpname,userPath.string());
            boost::filesystem::rename(boost::filesystem::path(tmpname), userPath, ec);
            if(!ec)
                return OK;
            std::cout<<"Error renaming tmp file"<<std::endl;
            return INTERNAL_ERROR;
        }
        std::cout<<"Error removing tmp file"<<std::endl;
        return INTERNAL_ERROR;
    }
    std::cout<<"Error opening file "<<userPath<<std::endl;

    return INTERNAL_ERROR;


}

responseType Server::updateName(std::string const& path, std::string const& newName){  //riceve in ingresso il vecchio path e il nuovo path

    std::cout<<"UPDATENAME"<<std::endl;

    const boost::filesystem::path oldp(userDirectory/boost::filesystem::path(path));
    const boost::filesystem::path newp(userDirectory/boost::filesystem::path(newName));

    //std::cout<<oldp<<std::endl;
    //std::cout<<newp<<std::endl;

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

    auto userPath = userDirectory / pathManipulation(path);

    if(!boost::filesystem::exists(userPath)){
        std::cout<<"Il path "<<userPath<<" non esiste!"<<std::endl;
        return NOT_PRESENT;
    }

    boost::system::error_code ec;
    boost::filesystem::remove(userPath,ec);  //questa versione di remove prende il codice errore, pertanto non c'è bisogno di inserirla in un try/catch
    if(ec){   //basta verificare la presenza dell'errore o meno
        std::cerr<<"Remove error: "<<ec.message()<<std::endl;
        return INTERNAL_ERROR;
    }
    std::cout<<"Remove success!"<<std::endl;

    return OK;
}

responseType Server::removeDir(std::string const& path){  //se path è un file, rimuove solo il file

    std::cout<<"REMOVEDIR"<<std::endl;

    auto userPath = userDirectory / pathManipulation(path);

    if(!boost::filesystem::exists(userPath)){
        std::cout<<"Il path "<<userPath<<" non esiste!"<<std::endl;
        return NOT_PRESENT;
    }

    boost::system::error_code ec;
    boost::filesystem::remove_all(userPath,ec);  //questa versione di remove_all prende il codice errore, pertanto non c'è bisogno di inserirla in un try/catch
    if(ec){   //basta verificare la presenza dell'errore o meno
        std::cerr<<"Remove dir error: "<<ec.message()<<std::endl;
        return INTERNAL_ERROR;
    }
    std::cout<<"Remove dir success!"<<std::endl;

    return OK;
}


responseType Server::createFile(std::string const& path, const std::vector<char>& recbuffer, const ssize_t& buffsize){

    std::cout<<"CREATEFILE"<<std::endl;

    auto userPath = userDirectory / pathManipulation(path);

    std::ofstream recfile;
    recfile.exceptions(std::ios_base::failbit | std::ios_base::badbit);  //tells the file to throw an exeption if those bit are set

    recfile.open(userPath.string(), std::ofstream::binary | std::ofstream::app);

    if(recfile.is_open()){
        try{
            recfile.write(recbuffer.data(),buffsize);
        }
        catch(std::exception& e){  //in caso di errore
            boost::system::error_code ec;
            std::cerr << "Write file error: " << e.what() << std::endl;
            boost::filesystem::remove(userPath,ec);  //rimuovi il file provvisorio
            if(!ec)
                return OK;
            std::cout<<"Error removing tmp file"<<std::endl;
            return INTERNAL_ERROR;
        }
        std::cout<<"Write file success!"<<std::endl;
        recfile.close();
        return OK;
    }
    std::cout<<"Error opening file "<<userPath<<std::endl;

    return INTERNAL_ERROR;
}

responseType Server::createDir(std::string const& path){

    std::cout<<"CREATEDIR"<<std::endl;

    auto userPath = userDirectory / pathManipulation(path);

    boost::system::error_code ec;
    boost::filesystem::create_directory(userPath,ec);  //questa versione di create_directory prende il codice errore, pertanto non c'è bisogno di inserirla in un try/catch
    if(ec){   //basta verificare la presenza dell'errore o meno
        std::cerr<<"Create dir error: "<<ec.message()<<std::endl;
        return INTERNAL_ERROR;
    }
    std::cout<<"Create dir success!"<<std::endl;

    return OK;

}

responseType Server::syncDir(std::string const& path){

    std::cout<<"SYNCDIR"<<std::endl;

    auto userPath = userDirectory / pathManipulation(path);

    if(boost::filesystem::exists(userPath)) {
        std::cout<<"Il path esiste!"<<std::endl;
        return OK;
    }
    std::cout<<"Il path non esiste!"<<std::endl;

    return NOT_PRESENT;
}

responseType Server::syncFile(std::string const& path, unsigned char* md_value, unsigned int md_len) {

    std::cout << "SYNCFILE" << std::endl;

    auto userPath = userDirectory / pathManipulation(path);

    //hash di <path>

    if (boost::filesystem::exists(userPath)) {
        unsigned char md_value_path[EVP_MAX_MD_SIZE];
        unsigned int md_len_path = computeHash(userPath.string(), md_value_path);
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