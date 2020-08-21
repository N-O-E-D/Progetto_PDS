//
// Created by lorenzo on 29/07/20.
//

#include "CryptoExecutor.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include <unistd.h>
#define BUF_SIZE 1024
#define KEYLEN 32
#define MAX_BUF 2048
unsigned int computeHash(const std::string &path,unsigned char md_value[]) {
    EVP_MD_CTX *md;
    //unsigned char md_value[EVP_MAX_MD_SIZE];

    int n,i;
    unsigned int md_len;
    unsigned char buf[BUF_SIZE];
    FILE *fin;

    if((fin = fopen(path.data(),"r")) == NULL) {
        std::cout<<"Couldnt open input file, try again\n";
        exit(1);
    }

    md = EVP_MD_CTX_new();
    EVP_MD_CTX_init(md);
    EVP_DigestInit(md, EVP_sha1());
    while((n = fread(buf,1,BUF_SIZE,fin)) > 0)
        EVP_DigestUpdate(md, buf,n);

    if(EVP_DigestFinal_ex(md, md_value, &md_len) != 1) {
        printf("Digest computation problem\n");
        exit(1);
    }
    printf("The digest is: ");
    for(i = 0; i < md_len; i++)
        printf("%02x", md_value[i]);
    printf("\n");
    EVP_MD_CTX_free(md);
    return md_len;
}

bool compareHash(unsigned char md_value1[],unsigned char md_value2[], int md_len){
    if(CRYPTO_memcmp(md_value1,md_value2, md_len)!=0)
        return false;
    return true;
}
std::pair<unsigned  char* , int> HKDF(std::string const& password,std::string const& salt){
    EVP_PKEY_CTX *pctx;
    unsigned char out[KEYLEN]; //la chiave deve avere lunghezza 32 bytes (poi uso aes-256)
    size_t outlen = sizeof(out);
    pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL);

    if (EVP_PKEY_derive_init(pctx) <= 0)
        printf("Error during EVP_PKEY_derive_init\n");
        if (EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256()) <= 0)
            printf("Error during EVP_PKEY_derive_init\n");
            if (EVP_PKEY_CTX_set1_hkdf_salt(pctx, salt.data(), salt.size()) <= 0)
                printf("Error during EVP_PKEY_CTX_set1_hkdf_salt\n");
                if (EVP_PKEY_CTX_set1_hkdf_key(pctx, password.data(), password.size()) <= 0)
                    printf("Error during EVP_PKEY_CTX_set1_hkdf_key\n");
                    if (EVP_PKEY_derive(pctx, out, &outlen) <= 0)
                            printf("Error during EVP_PKEY_derive\n");
    printf("La chiave è :\n");
    for (int i=0;i<KEYLEN;i++){
        printf("%02x",out[i]);
    }
    return std::make_pair(out,KEYLEN);
}

std::string genRandomBytes(int bytes){
    int i;
    //unsigned char random_string[MAX_BUF];
    std::string rb;
    rb.resize(bytes);
    if(bytes>MAX_BUF){
        printf("Maximum size allowed exxeced. Set to %d\n",MAX_BUF);
        bytes=MAX_BUF;
    }

    //RAND_bytes(random_string,bytes);
    //std::cout<<rb<<std::endl;
    printf("Sequence generated: ");
    for(i = 0; i < rb.size() ; i++)
        printf("%02x", rb.at(i));
    printf("\n");
    return rb;
}