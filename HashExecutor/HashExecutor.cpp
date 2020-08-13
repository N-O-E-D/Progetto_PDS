//
// Created by lorenzo on 29/07/20.
//

#include "HashExecutor.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include <unistd.h>
#define BUF_SIZE 1024

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
    std::cout<<&md_value<<std::endl;
    return md_len;

}