//
// Created by lorenzo on 29/07/20.
//

#pragma once
#include <string>
#include <array>
#include <openssl/evp.h>

unsigned int computeHash(std::string const& path,unsigned char md_value[]); //ritorna la lunghezza del digest
bool compareHash(unsigned char md_value1[],unsigned char md_value2[], int md_len);



