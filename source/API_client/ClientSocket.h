#pragma once

#include <vector>
#include <fstream>

#include <boost/asio.hpp>
#include "../../CryptoFunctions/CryptoExecutor.h"

enum messageType{
    UPDATE,UPDATE_NAME,REMOVE,/*REMOVE_DIR,*/CREATE_FILE,CREATE_DIR,SYNC_DIR,SYNC_FILE,AUTH,AUTH_CHALLENGE
};
enum responseType{
    OK,WRONG_USERNAME,WRONG_PASSWORD,CONNECTION_ERROR
};

class ClientSocket
{
public:
    using IoService = boost::asio::io_service;
    using TcpResolver = boost::asio::ip::tcp::resolver;
    using TcpResolverIterator = TcpResolver::iterator;
    using TcpSocket = boost::asio::ip::tcp::socket;

    ClientSocket(IoService& t_ioService, TcpResolverIterator t_endpointIterator);
    void update(std::string const& path, const std::function<void (std::string)> &action);
    void updateName(std::string const& path, std::string const& newName,const std::function<void (std::string)> &action);
    void remove(std::string const& path, const std::function<void (std::string)> &action);
    void createFile(std::string const& path, const std::function<void (std::string)> &action);
    void createDir(std::string const& path, const std::function<void (std::string)> &action);
    void syncDir(std::string const& path, const std::function<void (std::string)> &action);
    void syncFile(std::string const& path, const std::function<void (std::string)> &action);
    responseType authenticate(std:: string const& username, std::string const& password);
private:
    void openFile(std::string const& t_path);
    void doConnect();
    void doWriteFile(const boost::system::error_code& t_ec);
    template<class Buffer>
    void writeHeader(Buffer& t_buffer);
    template<class Buffer>
    void writeFileContent(Buffer& t_buffer);
    void buildHeader(messageType mt);
    void waitResponse(messageType mt,const std::function<void (std::string)> &action);
    void processResponse(size_t t_bytesTransferred,messageType mt,const std::function<void (std::string)> &action);
    void analyzeResponse(std::string response,messageType mt,const std::function<void (std::string)> &action);
    responseType waitChallenge();
    responseType doConnectSync();
    template<class Buffer>
    responseType writeSync(Buffer& t_buffer);
    template<class Buffer>
    responseType readUntilSync();
    responseType genCryptoChallenge();
    responseType waitResponseSync();
    responseType processResponseSync();
    TcpResolver m_ioService;
    TcpSocket m_socket;
    TcpResolverIterator m_endpointIterator;
    std::string m_password;
    std::string m_username;
    std::vector<char> m_buf;
    boost::asio::streambuf m_request;
    std::ifstream m_sourceFile;
    std::string m_path;
    std::string m_newName;
    std::string m_mdvalue;
    std::string m_iv;
    unsigned int m_mdlen;
    messageType m_messageType;
    boost::asio::streambuf m_response;
    std::string m_responseType;
    std::string m_cryptoChallenge;

    int m_fileSize;

};

enum logType{
    ERROR,TRACE
};
void log(logType lt,std::string const& message);