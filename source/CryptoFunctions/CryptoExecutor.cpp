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
#define DEBUG 0
#define DEPLOY 1

/**
 * Function which handles the OpenSSL errors
 */
void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}
/**
 * Function which computes the hash of a string
 */
unsigned int computeHash(const std::string &path,unsigned char md_value[]) {
    EVP_MD_CTX *md;

    int n;
    unsigned int md_len;
    unsigned char buf[BUF_SIZE];
    FILE *fin;

    if((fin = fopen(path.data(),"r")) == NULL) {
        log(CRYPTO,ERROR,"Couldnt open input file, try again");
        exit(1);
    }

    md = EVP_MD_CTX_new();
    EVP_MD_CTX_init(md);
    EVP_DigestInit(md, EVP_sha1());
    while((n = fread(buf,1,BUF_SIZE,fin)) > 0)
        EVP_DigestUpdate(md, buf,n);

    if(EVP_DigestFinal_ex(md, md_value, &md_len) != 1) {
        log(CRYPTO,ERROR,"Digest computation problem");
        exit(1);
    }
    log(CRYPTO,TRACE,"The digest is: ",std::string(md_value,md_value+md_len));
    EVP_MD_CTX_free(md);
    return md_len;
}
/**
 * Function which permits to compare two hashes
 */
bool compareHash(unsigned char md_value1[],unsigned char md_value2[], int md_len){
    if(CRYPTO_memcmp(md_value1,md_value2, md_len)!=0)
        return false;
    return true;
}
/**
 * Funchion which generates a strong key from password and salt
 */
std::vector<unsigned char> HKDF(std::string const& password,std::vector<unsigned char> const& salt){
    EVP_PKEY_CTX *pctx;
    unsigned char out[KEYLEN]; //la chiave deve avere lunghezza 32 bytes (poi uso aes-256)
    size_t outlen = sizeof(out);
    pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL);
    std::vector<unsigned char> result;
    result.resize(KEYLEN);
    if (EVP_PKEY_derive_init(pctx) <= 0)
        log(CRYPTO,ERROR,"Error during EVP_PKEY_derive_init");
    if (EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256()) <= 0)
        log(CRYPTO,ERROR,"Error during EVP_PKEY_derive_init");
    if (EVP_PKEY_CTX_set1_hkdf_salt(pctx, salt.data(), salt.size()) <= 0)
        log(CRYPTO,ERROR,"Error during EVP_PKEY_CTX_set1_hkdf_salt");
    if (EVP_PKEY_CTX_set1_hkdf_key(pctx, password.data(), password.size()) <= 0)
        log(CRYPTO,ERROR,"Error during EVP_PKEY_CTX_set1_hkdf_key");
    if (EVP_PKEY_derive(pctx, out, &outlen) <= 0)
        log(CRYPTO,ERROR,"Error during EVP_PKEY_derive");

    log(CRYPTO,TRACE,"La chiave è : ",result);
    return result;
}
/**
 * Function which generates a sequence of random bytes
 */
std::vector<unsigned char> genRandomBytes(int bytes){
    unsigned char random_string[MAX_BUF];
    std::vector<unsigned char> result;
    result.resize(bytes);
    if(bytes>MAX_BUF){
        log(CRYPTO,ERROR,"Maximum size allowed execed. Set to "+std::to_string(MAX_BUF));
        bytes=MAX_BUF;
    }

    RAND_bytes(random_string,bytes);

    for (int i=0;i<bytes;i++)
        result[i]=random_string[i];

    return result;
}
/**
 * Function which encypts a message beginning from key and iv
 */
std::vector<unsigned char> encrypt(std::string & message,std::vector<unsigned char> iv, std::vector<unsigned char> key){

    EVP_CIPHER_CTX *ctx;
    int len;
    /* A 256 bit key */
    /* A 128 bit IV */
    int ciphertext_len;
    unsigned char ciphertext[MAX_BUF];
    std::vector<unsigned char> result;
    std::string padding;
    padding.resize(28);
    for (int i=0;i<(int)padding.size();i++)
        padding[i]=0;
    message+=padding;
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
    result.resize(ciphertext_len);
    for (int i=0;i<ciphertext_len;i++)
        result[i]=ciphertext[i];
    log(CRYPTO,TRACE,"The ciphertext is : ",result);
    return result;
}
/**
 * Function which decrpyts a message beginning from key and iv
 */
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
    result.resize(plaintext_len-28);
    for (int i=0;i<plaintext_len-28;i++)
        result[i]=plaintext[i];
    log(CRYPTO,TRACE,"the plaintext is : ",result);
    return result;
}

void log(side s,logType lt,std::string const& message){
    if(s==SERVER || s==API_SERVER){
        switch(lt){
            case ERROR:
                BOOST_LOG_TRIVIAL(error) << message;
                break;
            case TRACE:
                BOOST_LOG_TRIVIAL(trace) << message;
                break;
        }
        return;
    }
#if DEBUG
    switch(lt){
        case ERROR:
            BOOST_LOG_TRIVIAL(error) << message;
            break;
        case TRACE:
            BOOST_LOG_TRIVIAL(trace) << message;
            break;
    }
#elif DEPLOY
    if(s==API_CLIENT || s==CRYPTO)
        return;
    switch(lt){
        case ERROR:
            std::cout<< "[ ERROR ] "+message<<std::endl;
            break;
        case TRACE:
            std::cout<<  "[ OK ] "+message<<std::endl;
            break;
    }
#endif
}
void log(side s,logType lt,std::string const& message1,std::vector<unsigned char> const& message2){
    if(s==SERVER || s==API_SERVER){
        switch(lt){
            case ERROR:
                BOOST_LOG_TRIVIAL(error) << message1;
                drawVectUnsChar(message2);
                break;
            case TRACE:
                BOOST_LOG_TRIVIAL(trace) << message1;
                drawVectUnsChar(message2);
                break;
        }
        return;
    }
#if DEBUG
    switch(lt){
        case ERROR:
            BOOST_LOG_TRIVIAL(error) << message1;
            drawVectUnsChar(message2);
            break;
        case TRACE:
            BOOST_LOG_TRIVIAL(trace) << message1;
            drawVectUnsChar(message2);
            break;
    }
#endif
}
void log(side s,logType lt,std::string const& message1,std::string const& message2){
    if(s==SERVER || s==API_SERVER){
        switch(lt){
            case ERROR:
                BOOST_LOG_TRIVIAL(error) << message1;
                drawStrToUnsChar(message2);
                break;
            case TRACE:
                BOOST_LOG_TRIVIAL(trace) << message1;
                drawStrToUnsChar(message2);
                break;
        }
        return;
    }
#if DEBUG
    switch(lt){
        case ERROR:
            BOOST_LOG_TRIVIAL(error) << message1;
            drawStrToUnsChar(message2);
            break;
        case TRACE:
            BOOST_LOG_TRIVIAL(trace) << message1;
            drawStrToUnsChar(message2);
            break;
    }
#endif
}
void log(side s,logType lt,std::string const& message, boost::asio::streambuf const& b){
    if(s==SERVER || s==API_SERVER){
        switch(lt){
            case ERROR:
                BOOST_LOG_TRIVIAL(error) << message;
                drawHeader(b);
                break;
            case TRACE:
                BOOST_LOG_TRIVIAL(trace) << message;
                drawHeader(b);
                break;
        }
    }
#if DEBUG
    switch(lt){
        case ERROR:
            BOOST_LOG_TRIVIAL(error) << message;
            drawHeader(b);
            break;
        case TRACE:
            BOOST_LOG_TRIVIAL(trace) << message;
            drawHeader(b);
            break;
    }
#endif
}
void drawVectUnsChar(std::vector<unsigned char> const& v){
    for (int i=0;i<(int)v.size();i++)
        printf("%02x",v[i]);
    printf("\n");
}
void drawStrToUnsChar(std::string const& s){
    for (int i=0;i<(int)s.size();i++)
        printf("%02x",(unsigned char)s[i]);
    printf("\n");
}
std::string vectUnsCharToStr(std::vector<unsigned char> const& v){
    std::string result;
    result.resize(v.size());
    for(int i=0;i<(int)v.size();i++)
        result[i]=(char) v[i];
    return result;
}
void drawHeader(boost::asio::streambuf const& s){
    auto bufs=s.data();
    std::cout<<"dimensione header : "<<s.size()<<std::endl;
    std::cout<<"HEADER"<<std::endl;
    std::cout<<std::string(boost::asio::buffers_begin(bufs),boost::asio::buffers_begin(bufs)+s.size());
    std::cout<<"FINE HEADER"<<std::endl;
}
