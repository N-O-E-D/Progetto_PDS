//
// Created by bruno on 29/08/20.
//

#ifndef SERVER_COOKIE_H
#define SERVER_COOKIE_H

#include <openssl/bn.h>
#include <chrono>
#include <iostream>

class Cookie {
    BIGNUM *data;
    std::chrono::seconds birthday;

public:
    Cookie(){
        this->data = BN_new();
        birthday = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
        std::cout<<"Cookie created"<<std::endl;
        std::cout<<"birthday: "<<birthday.count()<<std::endl;
    }

    Cookie(BIGNUM *num){
        this->data = BN_dup(num);
        birthday = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
        std::cout<<"Cookie created"<<std::endl;
        printInfo();
    }

    void update(){  //aggiorna solo "birthday"
        birthday = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
        std::cout<<"Cookie updated"<<std::endl;
        printInfo();

    }

    void update(BIGNUM *num){   //dato il BIGNUM come parametro, aggiorna "data" e "birthday"
        BN_copy(this->data,num);
        birthday = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
        std::cout<<"Cookie updated"<<std::endl;
        printInfo();
    }

    void printInfo(){
        std::cout<<"code: ";
        BN_print_fp(stdout,data);
        std::cout<<std::endl;
        std::cout<<"birthday: "<<birthday.count()<<std::endl;
    }

    bool is_expired(){
        if(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()) - birthday > std::chrono::seconds(0))
            return true;
        return false;
    }

    std::chrono::seconds getBirthday() const{
        return birthday;
    }

    BIGNUM *getData() const{
        return data;
    }

    ~Cookie(){
        delete data;
    }

};


#endif //SERVER_COOKIE_H
