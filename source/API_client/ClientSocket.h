#pragma once

#include <vector>
#include <fstream>

#include <boost/asio.hpp>
enum messageType{
    UPDATE,UPDATE_NAME,REMOVE,REMOVE_DIR,CREATE_FILE,CREATE_DIR
};

class ClientSocket
{
public:
    using IoService = boost::asio::io_service;
    using TcpResolver = boost::asio::ip::tcp::resolver;
    using TcpResolverIterator = TcpResolver::iterator;
    using TcpSocket = boost::asio::ip::tcp::socket;

    ClientSocket(IoService& t_ioService, TcpResolverIterator t_endpointIterator);

    void update(std::string const& path);
    void updateName(std::string const& path, std::string const& newName);
    void remove(std::string const& path);
    void removeDir(std::string const& path);
    void createFile(std::string const& path);
    void createDir(std::string const& path);

private:
    void openFile(std::string const& t_path);
    void doConnect();
    void doWriteFile(const boost::system::error_code& t_ec);
    template<class Buffer>
    void writeBuffer(Buffer& t_buffer);
    void buildHeader(messageType mt);

    TcpResolver m_ioService;
    TcpSocket m_socket;
    TcpResolverIterator m_endpointIterator;
    std::vector<char> m_buf;
    boost::asio::streambuf m_request;
    std::ifstream m_sourceFile;
    std::string m_path;
    std::string m_newName;
    messageType m_messageType;
    int m_fileSize;

};

