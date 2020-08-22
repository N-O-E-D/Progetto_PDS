#pragma once

#include <vector>
#include <fstream>

#include <boost/asio.hpp>
#include "../../CryptoFunctions/CryptoExecutor.h"

enum messageType{
    UPDATE,UPDATE_NAME,REMOVE,/*REMOVE_DIR,*/CREATE_FILE,CREATE_DIR,SYNC_DIR,SYNC_FILE,AUTH
};

class ClientSocket
{
public:
    using IoService = boost::asio::io_service;
    using TcpResolver = boost::asio::ip::tcp::resolver;
    using TcpResolverIterator = TcpResolver::iterator;
    using TcpSocket = boost::asio::ip::tcp::socket;

    ClientSocket(IoService& t_ioService, TcpResolverIterator t_endpointIterator);

    /* Modified by GIANDONATO FARINA */

    void update(std::string const& path, const std::function<void (std::string)> &action);
    void updateName(std::string const& path, std::string const& newName);
    void remove(std::string const& path, const std::function<void (std::string)> &action);
    //void removeDir(std::string const& path);
    void createFile(std::string const& path, const std::function<void (std::string)> &action);
    void createDir(std::string const& path, const std::function<void (std::string)> &action);
    void syncDir(std::string const& path, const std::function<void (std::string)> &action);
    void syncFile(std::string const& path, const std::function<void (std::string)> &action);
    void authenticate(std:: string const& username, std::string const& password);
private:
    void openFile(std::string const& t_path);
    void doConnect();
    void doWriteFile(const boost::system::error_code& t_ec);
    template<class Buffer>
    void writeHeader(Buffer& t_buffer);
    template<class Buffer>
    void writeFileContent(Buffer& t_buffer);
    void buildHeader(messageType mt);
    void waitResponse(messageType mt);
    void processResponse(size_t t_bytesTransferred,messageType mt);
    void analyzeResponse(std::string response,messageType mt);
    void doAuthentication();
    void waitChallenge();
    void sendCryptoChallenge();

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
    unsigned int m_mdlen;
    messageType m_messageType;
    boost::asio::streambuf m_response;
    std::string m_responseType;
    int m_fileSize;

};

