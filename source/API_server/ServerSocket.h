#pragma once

#include <array>
#include <vector>
#include <fstream>
#include <string>
#include <memory>
#include <boost/asio.hpp>
#include "../server/Server.h"
class Session
        : public std::enable_shared_from_this<Session>
{
public:
    using TcpSocket = boost::asio::ip::tcp::socket;

    Session(TcpSocket t_socket, Server server);

    void start()
    {
        doRead();
    }

private:
    void doRead();
    void processRead(size_t t_bytesTransferred);
    void createFile();
    void readData(std::istream &stream);
    void doReadFileContent(size_t t_bytesTransferred);
    void handleError(std::string const& t_functionName, boost::system::error_code const& t_ec);


    TcpSocket m_socket;
    std::vector<char> m_buf;
    boost::asio::streambuf m_requestBuf_;
    std::ofstream m_outputFile;
    size_t m_fileSize;
    std::string m_pathName;
    std::string m_messageType;
    std::string m_newName;
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

    ServerSocket(IoService& t_ioService, short t_port, std::string const& t_workDirectory, Server& server);

private:
    void doAccept();
    void createWorkDirectory();

    TcpSocket m_socket;
    TcpAcceptor m_acceptor;
    Server m_server;
    //std::string m_workDirectory;
};
