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

void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}

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
std::vector<unsigned char> HKDF(std::string const& password,std::vector<unsigned char> const& salt){
    EVP_PKEY_CTX *pctx;
    unsigned char out[KEYLEN]; //la chiave deve avere lunghezza 32 bytes (poi uso aes-256)
    size_t outlen = sizeof(out);
    pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL);
    std::vector<unsigned char> result;
    result.resize(KEYLEN);
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
    for (int i=0;i<KEYLEN;i++)
        result[i]=out[i];
    for (int i=0;i<KEYLEN;i++)
        printf("%02x",result[i]);
    printf("\n");
    return result;
}

std::vector<unsigned char> genRandomBytes(int bytes){
    int i;
    unsigned char random_string[MAX_BUF];
    std::vector<unsigned char> result;
    result.resize(bytes);
    if(bytes>MAX_BUF){
        printf("Maximum size allowed exxeced. Set to %d\n",MAX_BUF);
        bytes=MAX_BUF;
    }

    RAND_bytes(random_string,bytes);

    for (int i=0;i<bytes;i++)
        result[i]=random_string[i];

    return result;
}

std::vector<unsigned char> encrypt(std::string & message,std::vector<unsigned char> iv, std::vector<unsigned char> key){

    EVP_CIPHER_CTX *ctx;
    int len;
    /* A 256 bit key */
    /* A 128 bit IV */
    int ciphertext_len;
    unsigned char ciphertext[MAX_BUF];
    std::vector<unsigned char> result;
    std::string padding;
    std::cout<<message.size()<<std::endl;
    padding.resize(28);
    for (int i=0;i<padding.size();i++)
        padding[i]=0;
    message+=padding;
    std::cout<<message.size()<<std::endl;
    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();
    /*
     * Initialise the encryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data()))
        handleErrors();
    EVP_CIPHER_CTX_set_padding(ctx,0);
    /*
     * Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, (unsigned char*) message.data(),message.size()))
        handleErrors();
    ciphertext_len = len;
    /*
     * Finalise the encryption. Further ciphertext bytes may be written at
     * this stage.
     */
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        handleErrors();
    ciphertext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);
    printf("the ciphertext is:\n");
    result.resize(ciphertext_len);
    for (int i=0;i<ciphertext_len;i++)
        result[i]=ciphertext[i];
    for (int i=0;i<result.size();i++)
        printf("%02x",result[i]);
    printf("\n");
    return result;
}
std::vector<unsigned char> decrypt(std::vector<unsigned char> const& ciphertext,std::vector<unsigned char> iv,std::vector<unsigned char> key){
    EVP_CIPHER_CTX *ctx;

    int len;

    int plaintext_len;
    unsigned char plaintext[MAX_BUF];
    std::vector<unsigned char> result;
    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();
    /*
     * Initialise the decryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data()))
        handleErrors();
    EVP_CIPHER_CTX_set_padding(ctx,0);
    /*
     * Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary.
     */
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len,ciphertext.data(), ciphertext.size()))
        handleErrors();
    plaintext_len = len;

    /*
     * Finalise the decryption. Further plaintext bytes may be written at
     * this stage.
     */
    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
            handleErrors();

    plaintext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);
    printf("the plaintext length is: %d\n",plaintext_len-28);
    printf("the plaintext is:\n");
    result.resize(plaintext_len-28);
    for (int i=0;i<plaintext_len-28;i++)
        result[i]=plaintext[i];
    for (int i=0;i<result.size();i++)
        printf("%02x",result[i]);
    printf("\n");
    return result;
}