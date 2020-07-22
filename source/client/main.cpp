//
// Created by bruno on 14/07/2020.
//

#include <iostream>
#include <boost/rational.hpp>
#include <openssl/sha.h>

int main(){

    std::cout<<"Client\n";

    // Boost test
    boost::rational<int> a (2,3);
    std::cout<<"Prova boost: " << a << std::endl;

    // OpenSSL test
    unsigned char ibuf[] = "sha1 test";
    unsigned char obuf[20];

    SHA1(ibuf, 10, obuf);
    std::cout<<"SHA1: ";
    for (int i = 0; i < 20; i++) printf("%02x ", obuf[i]);

    return 0;
}