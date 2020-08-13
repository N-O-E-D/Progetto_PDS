#include <iostream>

#include <boost/asio/io_service.hpp>

#include "ClientSocket.h"

/*-------------------------------*/
#include <stdio.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <string>
#include <array>
#include <openssl/evp.h>
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
    return md_len;

}
/*-------------------------------------*/

int main(int argc, char* argv[])
{
    if (argc != 4) {
        std::cerr << "Usage: client <address> <port> <filePath>\n";
        return 1;
    }


    auto address = argv[1];
    auto port = argv[2];
    auto filePath = argv[3];

    try {
        boost::asio::io_service ioService;

        boost::asio::ip::tcp::resolver resolver(ioService);
        auto endpointIterator = resolver.resolve({ address, port });
        ClientSocket client(ioService, endpointIterator);
        unsigned char md_value[EVP_MAX_MD_SIZE];
        unsigned int md_len=computeHash(filePath,md_value);
        client.syncFile(filePath,md_value,md_len);
        ioService.run();

    } catch (std::fstream::failure& e) {
        std::cerr << e.what() << "\n";
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
