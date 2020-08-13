#include <iostream>

#include <boost/asio/read_until.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include "ServerSocket.h"

/*-----------------------------------------*/
#include <string>
#include <array>
#include <openssl/evp.h>
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
    return md_len;
}

bool compareHash(unsigned char md_value1[],unsigned char md_value2[], int md_len){
    if(CRYPTO_memcmp(md_value1, md_value2, md_len)!=0)
        return false;
    return true;
}
/*-----------------------------------------*/
Session::Session(TcpSocket t_socket)
        : m_socket(std::move(t_socket))
{
}


void Session::doRead()
{
    auto self = shared_from_this();
    async_read_until(m_socket, m_requestBuf_, "\n\n",
                     [this, self](boost::system::error_code ec, size_t bytes)
                     {
                         if (!ec)
                             processRead(bytes);
                         else
                             handleError(__FUNCTION__, ec);
                     });
}


void Session::processRead(size_t t_bytesTransferred)
{
    BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << "(" << t_bytesTransferred << ")"
                             << ", in_avail = " << m_requestBuf_.in_avail() << ", size = "
                             << m_requestBuf_.size() << ", max_size = " << m_requestBuf_.max_size() << ".";

    std::istream requestStream(&m_requestBuf_);
    readData(requestStream);
    if (m_messageType=="UPDATE" || m_messageType=="CREATE_FILE") {
        std::cout<<"dentro"<<std::endl;
        auto self = shared_from_this();
        m_buf.resize(m_fileSize);
        m_socket.async_read_some(boost::asio::buffer(m_buf.data(), m_fileSize),
                                 [this, self](boost::system::error_code ec, size_t bytes) {
                                     if (!ec)
                                         doReadFileContent(bytes);
                                 });
    }

}


void Session::readData(std::istream &stream)
{
    std::string b;

    stream >> m_messageType;
    stream >> m_pathName;
    if(m_messageType=="UPDATE_NAME")
        stream >> m_newName;
    if(m_messageType=="UPDATE" || m_messageType=="CREATE_FILE")
        stream >> m_fileSize;
    if (m_messageType=="SYNC_FILE"){
        stream >> m_mdvalue;
        //debug
        unsigned char md_value[EVP_MAX_MD_SIZE];
        unsigned int md_len=computeHash("/home/lorenzo/Scrivania/gg.txt",md_value);
        std::cout<<compareHash(md_value,reinterpret_cast<unsigned char*>(m_mdvalue.data()),md_len)<<std::endl;
    }
    //debug
    std::cout<< m_messageType<<std::endl;
    std::cout<< m_pathName<<std::endl;
    if(m_messageType=="UPDATE_NAME")
        std::cout<< m_newName<<std::endl;
    if(m_messageType=="UPDATE" || m_messageType=="CREATE_FILE")
        std::cout <<m_fileSize<<std::endl;
    if (m_messageType=="SYNC_FILE"){
        for (int i=0;i<m_mdvalue.size();i++)
            printf("%02x", (unsigned char)m_mdvalue[i]);
    }
        std::cout<<m_mdvalue<<std::endl;


    BOOST_LOG_TRIVIAL(trace) << m_pathName << " size is " << m_fileSize
                             << ", tellg = " << stream.tellg();
}


void Session::doReadFileContent(size_t t_bytesTransferred)
{
    if (t_bytesTransferred > 0) {
       m_file.insert(m_file.end(),m_buf.begin(),m_buf.end());
    }
    for (int i=0;i<m_file.size();i++)
        std::cout<<m_file.at(i);
    auto self = shared_from_this();
    m_socket.async_read_some(boost::asio::buffer(m_buf.data(), m_fileSize),
                             [this, self](boost::system::error_code ec, size_t bytes)
                             {
                                 doReadFileContent(bytes);
                             });
}


void Session::handleError(std::string const& t_functionName, boost::system::error_code const& t_ec)
{
    BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << " in " << t_functionName << " due to "
                             << t_ec << " " << t_ec.message() << std::endl;
}


ServerSocket::ServerSocket(IoService& t_ioService, short t_port, std::string const& t_workDirectory)
        : m_socket(t_ioService),
          m_acceptor(t_ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), t_port)),
          m_workDirectory(t_workDirectory)
{
    std::cout << "Server started\n";

    createWorkDirectory();

    doAccept();
}


void ServerSocket::doAccept()
{
    m_acceptor.async_accept(m_socket,
                            [this](boost::system::error_code ec)
                            {
                                if (!ec)
                                    std::make_shared<Session>(std::move(m_socket))->start();

                                doAccept();
                            });
}


void ServerSocket::createWorkDirectory()
{
    using namespace boost::filesystem;
    auto currentPath = path(m_workDirectory);
    if (!exists(currentPath) && !create_directory(currentPath))
        BOOST_LOG_TRIVIAL(error) << "Coudn't create working directory: " << m_workDirectory;
    current_path(currentPath);
}
