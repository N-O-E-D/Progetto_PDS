#pragma once

#include <array>
#include <vector>
#include <fstream>
#include <string>
#include <memory>
#include <boost/asio.hpp>
#include "../Server.h"
/**
 * functionType defines two different mode to use readAsyncUntil
 */
enum functionType{
    READ_FILE,DECRYPT_CRYPTO_CHALLENGE
};

class Session
        : public std::enable_shared_from_this<Session>
{
public:
    using TcpSocket = boost::asio::ip::tcp::socket;

    Session(TcpSocket t_socket, Server server);

    void start()
    {
        readAsyncUntil();
    }

private:
    /*++++++++++++++++ Private Method ++++++++++++++++++*/
    /*###### Communication methods ######*/
    void readAsyncUntil();
    void readAsyncSome(int dim,functionType ft);
    void doReadFileContent(size_t t_bytesTransferred);
    void sendToClient(responseType rt);
    void genChallenge();
    void waitCryptoChallenge();
    /*###### Analyze message method ######*/
    void processRead(size_t t_bytesTransferred);
    void readData(std::istream &stream);
    void parseAndDecryptCryptoChallenge();
    /*###### Error handling method ######*/
    void handleError(std::string const& t_functionName, boost::system::error_code const& t_ec);
    /*###### Server intercommunication method ######*/
    void manageMessage(std::string const& messageType);
    /*###### File manipulation methods ######*/
    int computeDimChunk();
    void createFile();
    /*++++++++++++++++++++ Instance Variables ++++++++++++++++++*/
    TcpSocket m_socket;
    std::shared_ptr<Session> self;
    std::vector<char> m_buf;
    boost::asio::streambuf m_requestBuf_;
    boost::asio::streambuf requestBuf;
    boost::asio::streambuf m_response;
    std::ofstream m_outputFile;
    size_t m_fileSize;
    std::string m_pathName;
    std::string m_messageType;
    std::string m_newName;
    std::string m_username;
    std::string m_challenge;
    std::string m_cryptoChallenge;
    std::string m_iv;
    std::string m_mdvalue;
    int m_chunks;
    int m_receivedChunks;
    std::vector<char> m_file;
    Server m_server;
    bool isAuthenticated;
};


class ServerSocket
{
public:
    using TcpSocket = boost::asio::ip::tcp::socket;
    using TcpAcceptor = boost::asio::ip::tcp::acceptor;
    using IoService = boost::asio::io_service;

    ServerSocket(IoService& t_ioService, short t_port, Server& server);

private:
    void doAccept();
    TcpSocket m_socket;
    TcpAcceptor m_acceptor;
    Server m_server;

};
