#pragma once

#include <array>
#include <vector>
#include <fstream>
#include <string>
#include <memory>
#include <boost/asio.hpp>
#include "../server/Server.h"
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
    void readAsyncUntil();
    void readAsyncSome(int dim,functionType ft);
    void processRead(size_t t_bytesTransferred);
    void createFile();
    void readData(std::istream &stream);
    void doReadFileContent(size_t t_bytesTransferred);
    void handleError(std::string const& t_functionName, boost::system::error_code const& t_ec);
    void manageMessage(std::string const& messageType);
    void sendToClient(responseType rt);
    void genChallenge();
    void waitCryptoChallenge();
    void parseAndDecryptCryptoChallenge();
    TcpSocket m_socket;
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
    std::vector<char> m_file;
    Server m_server;

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
enum logType{
    ERROR,TRACE
};
void log(logType lt,std::string const& message);
void log(logType lt,std::string const& message,boost::asio::streambuf const& s);
void log(logType lt,std::string const& message1,std::string const& message2);
void log(logType lt,std::string const& message1,std::vector<unsigned char> const& message2);
void drawHeader(boost::asio::streambuf const& s);
void drawStrToUnsChar(std::string const& s);
void drawVectUnsChar(std::vector<unsigned char> const& v);