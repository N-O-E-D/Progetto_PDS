#include <string>
#include <iostream>

#include <boost/filesystem/path.hpp>
#include <boost/log/trivial.hpp>

#include "ClientSocket.h"


ClientSocket::ClientSocket(IoService& t_ioService, TcpResolverIterator t_endpointIterator):
                            m_ioService(t_ioService),
                            m_socket(t_ioService),
                            m_endpointIterator(t_endpointIterator)
                            {}


void ClientSocket::openFile(std::string const& t_path)
{
    m_sourceFile.open(t_path, std::ios_base::binary);
    if (m_sourceFile.fail())
        throw std::fstream::failure("Failed while opening file " + t_path);
    else std::cout<<t_path + " aperto"<<std::endl;
    m_sourceFile.seekg(0, m_sourceFile.end);
    m_fileSize = m_sourceFile.tellg();
    m_sourceFile.seekg(0, m_sourceFile.beg);

}


void ClientSocket::doConnect()
{
    boost::asio::async_connect(m_socket, m_endpointIterator,
                               [this](boost::system::error_code ec, TcpResolverIterator)
                               {
                                   if (!ec) {
                                       //invio l'header
                                       writeHeader(m_request);
                                       //controllo anche se devo inviare il contenuto di un file
                                       if(m_messageType==UPDATE || m_messageType==CREATE_FILE)
                                           doWriteFile(ec);

                                   } else {
                                       std::cout << "Couldn't connect to host. Please run server "
                                                    "or check network connection.\n";
                                       BOOST_LOG_TRIVIAL(error) << "Error: " << ec.message();
                                   }
                               });
}


void ClientSocket::doWriteFile(const boost::system::error_code& t_ec)
{
    if (!t_ec) {
        if (m_sourceFile) {
            m_buf.resize(m_fileSize);
            m_sourceFile.read(m_buf.data(), m_fileSize);
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
            auto buf = boost::asio::buffer(m_buf.data(), static_cast<size_t>(m_sourceFile.gcount()));
            writeFileContent(buf);
        }
    } else {
        BOOST_LOG_TRIVIAL(error) << "Error: " << t_ec.message();
    }
}
void ClientSocket::buildHeader(messageType mt){
    std::ostream requestStream(&m_request);
    switch (mt) {
        case UPDATE:
            requestStream << "UPDATE\n" << m_path << "\n" << std::to_string(m_fileSize) << "\n\n";
            break;
        case UPDATE_NAME:
            requestStream << "UPDATE_NAME\n" << m_path << "\n" <<m_newName<< "\n\n";
            break;
        case REMOVE:
            requestStream << "REMOVE\n" << m_path << "\n" << "\n\n";
            break;
        case REMOVE_DIR:
            requestStream << "REMOVE_DIR\n" << m_path << "\n" << "\n\n";
            break;
        case CREATE_FILE:
            requestStream << "CREATE_FILE\n" << m_path << "\n" <<std::to_string(m_fileSize)<<"\n\n";
            break;
        case CREATE_DIR:
            requestStream << "CREATE_DIR\n" << m_path<<"\n\n";
            break;
        case SYNC_DIR:
            requestStream << "SYNC_DIR\n" << m_path<<"\n\n";
            break;
        case SYNC_FILE:
            requestStream << "SYNC_FILE\n" << m_path<<"\n"<<m_mdvalue<<"\n\n";
            break;
        default:
            return;
            break;
    }
    //debug
    auto bufs=m_request.data();
    std::cout<<"HEADER"<<std::endl;
    std::cout<<std::string(boost::asio::buffers_begin(bufs),boost::asio::buffers_begin(bufs)+m_request.size());
    std::cout<<"FINE HEADER"<<std::endl;
    BOOST_LOG_TRIVIAL(trace) << "Request size: " << m_request.size();
    }

void ClientSocket::update(const std::string &path) {
    m_path=path;
    m_messageType=UPDATE;
    openFile(m_path);
    buildHeader(UPDATE);
    doConnect();
}
void ClientSocket::updateName(const std::string &path,std::string const& newName) {
    m_path=path;
    m_messageType=UPDATE_NAME;
    m_newName=newName;
    buildHeader(UPDATE_NAME);
    doConnect();
}
void ClientSocket::remove(const std::string &path) {
    m_path=path;
    m_messageType=REMOVE;
    buildHeader(REMOVE);
    doConnect();
}
void ClientSocket::removeDir(const std::string &path) {
    m_path=path;
    m_messageType=REMOVE_DIR;
    buildHeader(REMOVE_DIR);
    doConnect();
}
void ClientSocket::createFile(const std::string &path) {
    m_path=path;
    m_messageType=CREATE_FILE;
    openFile(m_path);
    buildHeader(CREATE_FILE);
    doConnect();
}
void ClientSocket::createDir(const std::string &path) {
    m_path=path;
    m_messageType=CREATE_DIR;
    buildHeader(CREATE_DIR);
    doConnect();
}
void ClientSocket::syncDir(std::string const& path){
    m_path=path;
    m_messageType=SYNC_DIR;
    buildHeader(SYNC_DIR);
    doConnect();
}
void ClientSocket::syncFile(std::string const& path,unsigned char* md_value,unsigned int md_len){
    m_path=path;
    m_messageType=SYNC_FILE;
    printf("digest: ");
    for(int i = 0; i < md_len; i++)
        printf("%02x", md_value[i]);
    printf("\n");
    std::string sName(reinterpret_cast<char* >(md_value),(size_t) md_len);
    m_mdvalue=sName;
    for(int i = 0; i < md_len; i++)
        printf("%02x", (unsigned char)sName[i]);
    buildHeader(SYNC_FILE);
    doConnect();
}
template<class Buffer>
void ClientSocket::writeHeader(Buffer& t_buffer)
{
    boost::asio::async_write(m_socket,
                             t_buffer,
                             [this](boost::system::error_code ec, size_t )
                             {
                                std::cout<<"header inviato"<<std::endl;
                                });
                             };

template<class Buffer>
void ClientSocket::writeFileContent(Buffer& t_buffer)
{
    boost::asio::async_write(m_socket,
                             t_buffer,
                             [this](boost::system::error_code ec, size_t )
                             {
                                 std::cout<<"file inviato"<<std::endl;
                             });
    };

