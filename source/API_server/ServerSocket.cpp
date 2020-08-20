#include <iostream>

#include <boost/asio/read_until.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include "ServerSocket.h"


Session::Session(TcpSocket t_socket,Server server)
        : m_socket(std::move(t_socket)),
        m_server(server)
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
        std::cout<<"Sto leggendo il contenuto del file..."<<std::endl;
        auto self = shared_from_this();
        m_buf.resize(m_fileSize);
        std::cout << "dimensione buffer : " << m_buf.size()<<std::endl;
        m_socket.async_read_some(boost::asio::buffer(m_buf.data(), m_fileSize),
                                 [this, self](boost::system::error_code ec, size_t bytes) {
                                     if (!ec)
                                         doReadFileContent(bytes);
                                 });
    }
    else manageMessage(m_messageType);
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
    if (m_messageType=="SYNC_FILE")
        stream >> m_mdvalue;
        
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
    std::cout<<"ho ricevuto questo file: "<<std::endl;
    for (int i=0;i<m_file.size();i++)
        std::cout<<m_file.at(i);

    manageMessage(m_messageType);

}
void Session::manageMessage(std::string const& messageType){
    std::cout<<"Invio al server "<<messageType<<std::endl;
    responseType rt;
    //ora che ho tutti i dati ricevuti dal client richiamo le funzioni fornite dalla classe Server a seconda dell'azione
    if (m_messageType=="UPDATE")
        rt=m_server.update(m_pathName,m_file,m_fileSize);
    if (m_messageType=="UPDATE_NAME")
        rt=m_server.updateName(m_pathName,m_newName);
    if (m_messageType=="REMOVE")
        rt=m_server.removeDir(m_pathName);
    /*if (m_messageType=="REMOVE_DIR")
        rt=m_server.removeDir(m_pathName);*/
    if (m_messageType=="CREATE_FILE")
        rt=m_server.createFile(m_pathName,m_file,m_fileSize);
    if (m_messageType=="CREATE_DIR")
        rt=m_server.createDir(m_pathName);
    if (m_messageType=="SYNC_DIR")
        rt=m_server.syncDir(m_pathName);
    if (m_messageType=="SYNC_FILE")
        rt=m_server.syncFile(m_pathName,(unsigned char*) m_mdvalue.data(),(unsigned int)m_mdvalue.size());

    sendToClient(rt);


}
void Session::sendToClient(responseType rt){

    std::ostream responseStream(&m_response);
    switch (rt){
        case OK:
            responseStream << "OK"<<"\n\n";
            break;
        case NOT_PRESENT:
            responseStream << "NOT_PRESENT"<<"\n\n";
            break;
        case OLD_VERSION:
            responseStream << "OLD_VERSION"<<"\n\n";
            break;
        case INTERNAL_ERROR:
            responseStream << "INTERNAL_ERROR"<<"\n\n";
            break;
    }
    boost::asio::async_write(m_socket,
                              m_response,
                             [this](boost::system::error_code ec, size_t )
                             {
                                 std::cout<<"messaggio di risposta inviato"<<std::endl;
                                 //doAccept();
                             });

}
void Session::handleError(std::string const& t_functionName, boost::system::error_code const& t_ec)
{
    BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << " in " << t_functionName << " due to "
                             << t_ec << " " << t_ec.message() << std::endl;
}


ServerSocket::ServerSocket(IoService& t_ioService, short t_port, Server& server)
        : m_socket(t_ioService),
          m_acceptor(t_ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), t_port)),
          m_server(server)
{
    std::cout << "Server started\n";

    doAccept();
}


void ServerSocket::doAccept()
{
    std::cout<<"server in ascolto"<<std::endl;
    m_acceptor.async_accept(m_socket,
                            [this](boost::system::error_code ec)
                            {
                                if (!ec) {
                                    std::cout<<"Comunicazione accettata\n";
                                    std::make_shared<Session>(std::move(m_socket), std::move(m_server))->start();
                                }

                                doAccept();
                            });
}
