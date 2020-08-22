//
// Created by lorenzo on 29/07/20.
//

#pragma once
#include <string>
#include <array>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/rand.h>
#include <openssl/conf.h>
#include <openssl/err.h>

unsigned int computeHash(std::string const& path,unsigned char md_value[]); //ritorna la lunghezza del digest
bool compareHash(unsigned char md_value1[],unsigned char md_value2[], int md_len);
std::pair<unsigned char*,int>  HKDF(std::string const& password, std::string const& salt);
unsigned char* genRandomBytes(int bytes);
std::pair<unsigned char*,int> encrypt(std::string const& message,unsigned char* iv,unsigned char* key);
void handleErrors(void);