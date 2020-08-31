//
// Created by bruno on 29/08/20.
//

#ifndef SERVER_COOKIE_H
#define SERVER_COOKIEMAP_H

#include "Server.h"
#include <openssl/bn.h>
#include <chrono>
#include <iostream>
#include <shared_mutex>
#include <map>

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

//mutex e locks per thread safety
typedef std::shared_mutex Lock;
typedef std::unique_lock<Lock> WriteLock;
typedef std::shared_lock<Lock> ReadLock;

class CookieMap {
    std::map<std::string,Cookie> userCookies;
    Lock cookielock;

public:
    responseType insertCookie(const std::string username, Cookie& cookie){
        WriteLock w_lock(cookielock);

        if(userCookies.find(username) != userCookies.end()) {  //se esiste la chiave username
            userCookies[username] = cookie;
            return OK;
        }
        else{
            std::cout<<"Username non esistente"<<std::endl;
            return WRONG_USERNAME;
        }
    }

    responseType getCookie(const std::string username, Cookie& cookie){
        ReadLock r_lock(cookielock);

        if(userCookies.find(username) != userCookies.end()) {  //se esiste la chiave username
            cookie = userCookies.at(username);
            return OK;
        }
        else{
            std::cout<<"Username non esistente"<<std::endl;
            return WRONG_USERNAME;
        }
    }

    responseType updateCookie(const std::string username){  //aggiorna solo "birthday" del cookie
        WriteLock w_lock(cookielock);

        if(userCookies.find(username) != userCookies.end()) {  //se esiste la chiave username
            Cookie newcookie;
            getCookie(username,newcookie);
            newcookie.update();
            userCookies[username] = newcookie;
            return OK;
        }
        else{
            std::cout<<"Username non esistente"<<std::endl;
            return WRONG_USERNAME;
        }
    }

    responseType updateCookie(const std::string username, const Cookie& cookie){  //aggiorna "data" e "birthday" del cookie
        WriteLock w_lock(cookielock);

        if(userCookies.find(username) != userCookies.end()) {  //se esiste la chiave username
            Cookie newcookie;
            getCookie(username,newcookie);
            newcookie.update(cookie.getData());
            userCookies[username] = newcookie;
            return OK;
        }
        else{
            std::cout<<"Username non esistente"<<std::endl;
            return WRONG_USERNAME;
        }
    }

    responseType isValid(const std::string username, const Cookie& cookie){
        ReadLock r_lock(cookielock);

        if(userCookies.find(username) != userCookies.end()) {  //se esiste la chiave username
            if(userCookies.at(username).is_expired())
                return EXPIRED;
            return OK;
        }
        else{
            std::cout<<"Username non esistente"<<std::endl;
            return WRONG_USERNAME;
        }
    }
};

#endif //SERVER_COOKIE_H
