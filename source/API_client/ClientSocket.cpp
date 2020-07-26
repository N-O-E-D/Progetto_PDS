#include <string>
#include <iostream>

#include <boost/filesystem/path.hpp>
#include <boost/log/trivial.hpp>

#include "ClientSocket.h"


ClientSocket::ClientSocket(IoService& t_ioService, TcpResolverIterator t_endpointIterator
               /*,std::string const& t_path*/)
        : m_ioService(t_ioService), m_socket(t_ioService),
          m_endpointIterator(t_endpointIterator)/*, m_path(t_path)*/
{
    //doConnect();
    //openFile(m_path);
}


void ClientSocket::openFile(std::string const& t_path)
{
    m_sourceFile.open(t_path, std::ios_base::binary);
    if (m_sourceFile.fail())
        throw std::fstream::failure("Failed while opening file " + t_path);

    m_sourceFile.seekg(0, m_sourceFile.end);
    auto fileSize = m_sourceFile.tellg();
    m_buf.resize(fileSize);
    m_sourceFile.seekg(0, m_sourceFile.beg);
    std::ostream requestStream(&m_request);
    //costruisco l'header
    requestStream << m_messageType<< "\n" << t_path<<"\n"<<std::to_string(fileSize) << "\n\n";
    //debug
    auto bufs=m_request.data();
    std::cout<<"HEADER"<<std::endl;
    std::cout<<std::string(boost::asio::buffers_begin(bufs),boost::asio::buffers_begin(bufs)+m_request.size());
    std::cout<<"FINE HEADER"<<std::endl;
    BOOST_LOG_TRIVIAL(trace) << "Request size: " << m_request.size();
}


void ClientSocket::doConnect()
{
    boost::asio::async_connect(m_socket, m_endpointIterator,
                               [this](boost::system::error_code ec, TcpResolverIterator)
                               {
                                   if (!ec) {
                                       //invio l'header
                                       writeBuffer(m_request);
                                   } else {
                                       std::cout << "Coudn't connect to host. Please run server "
                                                    "or check network connection.\n";
                                       BOOST_LOG_TRIVIAL(error) << "Error: " << ec.message();
                                   }
                               });
}


void ClientSocket::doWriteFile(const boost::system::error_code& t_ec)
{
    if (!t_ec) {
        if (m_sourceFile) {

            m_sourceFile.read(m_buf.data(), m_buf.size());
            if (m_sourceFile.fail() && !m_sourceFile.eof()) {
                auto msg = "Failed while reading file";
                BOOST_LOG_TRIVIAL(error) << msg;
                throw std::fstream::failure(msg);
            }
            std::stringstream ss;
            ss << "Send " << m_sourceFile.gcount() << " bytes, total: "
               << m_sourceFile.tellg() << " bytes";
            BOOST_LOG_TRIVIAL(trace) << ss.str();
            std::cout<<m_buf.data()<<std::endl;
            auto buf = boost::asio::buffer(m_buf.data(), static_cast<size_t>(m_buf.size()));
            //writeBuffer(buf);
        }
    } else {
        BOOST_LOG_TRIVIAL(error) << "Error: " << t_ec.message();
    }
}
void ClientSocket::sendMessage(messageType mt, const std::string &path) {
    m_path=path;

    switch (mt){
        case (UPDATE):
            m_messageType="UPDATE";
            break;
        case(UPDATE_NAME):
            m_messageType="UPDATE_NAME";
            break;
        case(REMOVE):
            m_messageType="REMOVE";
            break;
        case(REMOVE_DIR):
            m_messageType="REMOVE_DIR";
            break;
        case (INSERT):
            m_messageType="INSERT";
            break;
    }
    doConnect();
    openFile(m_path);

}

template<class Buffer>
void ClientSocket::writeBuffer(Buffer& t_buffer)
{
    boost::asio::async_write(m_socket,
                             t_buffer,
                             [this](boost::system::error_code ec, size_t )
                             {
                                 doWriteFile(ec);
                             });
}
void ClientSocket::setNewName(const std::string& newName){
    m_newName=newName;
}