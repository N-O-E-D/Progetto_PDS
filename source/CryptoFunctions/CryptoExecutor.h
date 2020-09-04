//
// Created by lorenzo on 29/07/20.
//

#pragma once
#include <string.h>
#include <array>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/rand.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <vector>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

unsigned int computeHash(std::string const& path,unsigned char md_value[]); //ritorna la lunghezza del digest
bool compareHash(unsigned char md_value1[],unsigned char md_value2[], int md_len);
std::vector<unsigned char>  HKDF(std::string const& password, std::vector<unsigned char> const& salt);
std::vector<unsigned char> genRandomBytes(int bytes);
std::vector<unsigned char> encrypt(std::string& message,std::vector<unsigned char> iv,std::vector<unsigned char> key);
std::vector<unsigned char> decrypt(std::vector<unsigned char> const& ciphertext,std::vector<unsigned char> iv,std::vector<unsigned char> key);
void handleErrors(void);


enum logType{
    ERROR,TRACE
};
void log(logType lt,std::string const& message);
void log(logType lt,std::string const& message1,std::vector<unsigned char> const& message2);
void log(logType lt,std::string const& message1,std::string const& message2);
void log(logType lt,std::string const& message,boost::asio::streambuf const& s);
void drawVectUnsChar(std::vector<unsigned char> const& v);
void drawStrToUnsChar(std::string const& s);
void drawHeader(boost::asio::streambuf const& s);
std::string vectUnsCharToStr(std::vector<unsigned char> const& v);