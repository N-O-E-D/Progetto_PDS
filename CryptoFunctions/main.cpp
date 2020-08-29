#include "CryptoExecutor.h"
#include <iostream>
int main(){

    //unsigned char md_value[EVP_MAX_MD_SIZE];
    //unsigned int md_len=computeHash("/home/lorenzo/Scrivania/prova.txt",md_value);
    //printf("The digest is: ");
    //for(int i = 0; i < md_len; i++)
    //    printf("%02x", md_value[i]);
    //printf("\n");
    /*std::vector<unsigned char> iv=genRandomBytes(16);
    std::vector<unsigned char> key= genRandomBytes(32);
    std::vector<unsigned char> m=genRandomBytes(100);
    for (int i=0;i<m.size();i++)
        printf("%02x",m[i]);
    std::string s;
    s.resize(m.size());
    for (int i=0;i<m.size();i++)
        s[i]=(char)m[i];
    std::vector<unsigned char>  ff=encrypt(s,iv,key);
    std::vector<unsigned char>  key2=genRandomBytes(32);
    std::vector<unsigned char> ff_dec=decrypt(ff,iv,key2);
    if(CRYPTO_memcmp(m.data(),ff_dec.data(),m.size() )!=0)
        std::cout<<"diversi"<<std::endl;
    else std::cout<<"uguali"<<std::endl;*/
    //std::vector<unsigned char> hdf=HKDF("ciaoooo",genRandomBytes(32));
    return 0;
}
