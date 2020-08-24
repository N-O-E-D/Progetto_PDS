#include "CryptoExecutor.h"
#include <iostream>
int main(){

    //unsigned char md_value[EVP_MAX_MD_SIZE];
    //unsigned int md_len=computeHash("/home/lorenzo/Scrivania/prova.txt",md_value);
    //printf("The digest is: ");
    //for(int i = 0; i < md_len; i++)
    //    printf("%02x", md_value[i]);
    //printf("\n");
    std::vector<unsigned char> iv=genRandomBytes(16);
    std::vector<unsigned char> key= genRandomBytes(32);
    std::vector<unsigned char>  ff=encrypt("budello infame",iv,key);
    std::string message;
    message.resize(ff.size());
    for (int i=0;i<message.size();i++)
        message[i] = (char) ff[i];
    std::vector<unsigned char> ff_dec=decrypt(message,iv,key);
    //std::vector<unsigned char> hdf=HKDF("ciaoooo",genRandomBytes(32));
    return 0;
}
