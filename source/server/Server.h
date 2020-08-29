//
// Created by bruno on 25/07/20.
//

#ifndef PROVA_SOCKET_ASIO_SERVER_H
#define PROVA_SOCKET_ASIO_SERVER_H

#include "../../CryptoFunctions/CryptoExecutor.h"
#include "Cookie.h"
#include "openssl/bn.h"
#include <shared_mutex>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/system/error_code.hpp>
#include <boost/filesystem/operations.hpp>  //include boost::filesystem::path.hpp
#include <boost/filesystem/fstream.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/iterator.hpp>
#include <boost/cstdint.hpp>
#include <fstream>

enum responseType{
    OK, NOT_PRESENT, OLD_VERSION, INTERNAL_ERROR, WRONG_PASSWORD, WRONG_USERNAME, CHALLENGE , NON_AUTHENTICATED, EXPIRED
};

responseType loadUsers(const std::string& filename);

//Affinchè vada bene per tutti, metto un path relativo. Nella versione definitiva dovrebbe essere assoluto, per la singola macchina.
static std::string workingdirectory("../backup");

class Server {

public:

    boost::filesystem::path userDirectory;

    void setUserDirectory(const std::string& username);
    responseType update(std::string const& path, const std::vector<char>& recbuffer, const ssize_t& buffsize);
    responseType updateName(std::string const& path, std::string const& newName);
    responseType remove(std::string const& path);
    responseType removeDir(std::string const& path);
    responseType createFile(std::string const& path, const std::vector<char>& recbuffer, const ssize_t& buffsize);
    responseType createDir(std::string const& path);
    responseType syncDir(std::string const& path);
    responseType syncFile(std::string const& path,unsigned char* md_value,unsigned int md_len);

    responseType checkCredenziali(const std::string& username, const std::string& password);
    responseType checkCredenziali(const std::string& username);
    responseType UserToPassword(const std::string& username, std::string& password);

    responseType insertCookie(const std::string username, Cookie& cookie);
    responseType getCookie(const std::string username, Cookie& cookie);
    responseType updateCookie(const std::string username);
    responseType updateCookie(const std::string username, const Cookie& cookie);
    responseType isValid(const std::string username, const Cookie& cookie);
};



#endif //PROVA_SOCKET_ASIO_SERVER_H
