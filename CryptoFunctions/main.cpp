#include "CryptoExecutor.h"
#include <iostream>
int main(){

    //unsigned char md_value[EVP_MAX_MD_SIZE];
    //unsigned int md_len=computeHash("/home/lorenzo/Scrivania/prova.txt",md_value);
    //printf("The digest is: ");
    //for(int i = 0; i < md_len; i++)
    //    printf("%02x", md_value[i]);
    //printf("\n");
    unsigned char* iv=genRandomBytes(16);
    unsigned char* key= genRandomBytes(32);
    encrypt("bbbbbbbb",iv,key);
    return 0;
}

