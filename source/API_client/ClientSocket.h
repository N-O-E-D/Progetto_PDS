#pragma once

#include <vector>
#include <fstream>

#include <boost/asio.hpp>

enum messageType{
    UPDATE, UPDATE_NAME,REMOVE,REMOVE_DIR,INSERT
};

class ClientSocket
{
public:
    using IoService = boost::asio::io_service;
    using TcpResolver = boost::asio::ip::tcp::resolver;
    using TcpResolverIterator = TcpResolver::iterator;
    using TcpSocket = boost::asio::ip::tcp::socket;

    ClientSocket(IoService& t_ioService, TcpResolverIterator t_endpointIterator/*,
           std::string const& t_path*/);

    void sendMessage(messageType mt, std::string const& path);
    void setNewName(const std::string& newName);
private:
    void openFile(std::string const& t_path);
    void doConnect();
    void doWriteFile(const boost::system::error_code& t_ec);
    template<class Buffer>
    void writeBuffer(Buffer& t_buffer);


    TcpResolver m_ioService;
    TcpSocket m_socket;
    TcpResolverIterator m_endpointIterator;
    std::vector<char> m_buf;
    std::string m_messageType;
    std::string m_newName;
    boost::asio::streambuf m_request;
    std::ifstream m_sourceFile;
    std::string m_path;
};

