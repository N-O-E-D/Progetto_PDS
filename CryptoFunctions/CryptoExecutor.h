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
unsigned int computeHash(std::string const& path,unsigned char md_value[]); //ritorna la lunghezza del digest
bool compareHash(unsigned char md_value1[],unsigned char md_value2[], int md_len);
std::vector<unsigned char>  HKDF(std::string const& password, std::vector<unsigned char> const& salt);
std::vector<unsigned char> genRandomBytes(int bytes);
std::vector<unsigned char> encrypt(std::string const& message,std::vector<unsigned char> iv,std::vector<unsigned char> key);
std::vector<unsigned char> decrypt(std::string const& ciphertext,std::vector<unsigned char> iv,std::vector<unsigned char> key);
void handleErrors(void);