#include <iostream>

#include <boost/asio/read_until.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include "ServerSocket.h"

#define LENGTHCHALLENGE 100

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
    //debug
    auto bufs=m_requestBuf_.data();
    std::cout<<"dimensione bufRequest : "<<m_requestBuf_.size()<<std::endl;
    std::cout<<"HEADER"<<std::endl;
    std::cout<<std::string(boost::asio::buffers_begin(bufs),boost::asio::buffers_begin(bufs)+m_requestBuf_.size());
    std::cout<<"FINE HEADER"<<std::endl;
    //fine debug
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
    else if(m_messageType=="AUTH") {
        if(m_server.checkCredenziali(m_username)==OK)
            genChallenge();
        else sendToClient(WRONG_USERNAME);
    }
    else if(m_messageType=="AUTH_CHALLENGE"){
        std::cout<<"Sto leggendo l'iv e la challenge cifrata..."<<std::endl;
        auto self = shared_from_this();
        m_buf.clear();
        m_buf.resize(m_iv.size()+m_cryptoChallenge.size());
        std::cout<<m_buf.size()<<std::endl;
        m_socket.async_read_some(boost::asio::buffer(m_buf.data(), m_buf.size()),
                                 [this, self](boost::system::error_code ec, size_t bytes) {
                                     if (!ec)
                                         parseAndDecryptCryptoChallenge();
                                     else std::cout<<"Errore nella lettura dell'iv e della challenge cifrata : "<<ec.message()<<std::endl;
                                 });
    }
    else manageMessage(m_messageType);
}
void Session::parseAndDecryptCryptoChallenge(){
    //debug
    printf("ho ricevuto: \n");
    for (int i=0;i<m_iv.size()+m_cryptoChallenge.size();i++)
        printf("%02x",(unsigned char)m_buf[i]);
    printf("\n");
    //fine debug
    std::vector<unsigned char> iv_vect;
    std::vector<unsigned char> cc_vect;
    cc_vect.resize(m_cryptoChallenge.size());
    iv_vect.resize(m_iv.size());
    for (int i=0;i<m_iv.size();i++)
        iv_vect[i]=(unsigned char) m_buf[i];
    for (int i=m_iv.size();i<m_iv.size()+m_cryptoChallenge.size();i++)
        cc_vect[i-m_iv.size()]=(unsigned char) m_buf[i];
    //debug
    printf("Ho letto iv: \n");
    for (int i=0;i<m_iv.size();i++)
        printf("%02x",iv_vect[i]);
    printf("\n");
    printf("Ho letto la sfida cifrata: \n");
    for (int i=0;i<cc_vect.size();i++)
        printf("%02x",cc_vect[i]);
    printf("\n");
    //fine debug
    std::string password;
    if (m_server.UserToPassword(m_username,password)==WRONG_USERNAME){
        sendToClient(WRONG_USERNAME);
        return;
    }
    std::vector<unsigned char> key=HKDF(password, iv_vect);
    std::cout<<"Lunghezza chiave : "<<key.size()<<std::endl;
    std::vector<unsigned char> dec_challenge=decrypt(cc_vect,iv_vect,key);
    if(CRYPTO_memcmp((unsigned char*)m_challenge.data(),dec_challenge.data(),LENGTHCHALLENGE)==0) {
        std::cout << "AUTENTICAZIONE RIUSCITA" << std::endl;
        sendToClient(OK);
    }
    else {
        std::cout<<"AUTENTICAZIONE FALLITA"<<std::endl;
        sendToClient(WRONG_PASSWORD);
    }

}
void Session::genChallenge(){
    std::vector<unsigned char> challenge = genRandomBytes(LENGTHCHALLENGE);
    //debug
    printf("La challenge generata è: \n");

    //fine debug

    m_challenge.resize(challenge.size());
    for (int i=0;i<challenge.size();i++)
        m_challenge[i]=challenge[i];
    for (int i=0;i<m_challenge.size();i++)
        printf("%02x",m_challenge[i]);
    printf("\n");
    auto buf = boost::asio::buffer(m_challenge.data(), LENGTHCHALLENGE);
    auto self=shared_from_this();
    boost::asio::async_write(m_socket,
                             buf,
                             [this,self](boost::system::error_code ec, size_t bytes)
                             {
                                 std::cout<<"challenge inviata, aspetto la challenge cifrata..."<<std::endl;
                                 async_read_until(m_socket,m_requestBuf_, "\n\n",
                                                  [this,self](boost::system::error_code ec, size_t bytes)
                                                  {
                                                      if (!ec) {
                                                          std::cout<<"ho ricevuto la sfida cifrata, ora elaboro..."<<std::endl;
                                                          processRead(bytes);
                                                      }
                                                      else
                                                          //handleError(__FUNCTION__, ec);
                                                          std::cout<<"Errore nella ricezione della sfida cifrata : "<<ec.message()<<std::endl;
                                                  });
                             });
}

void Session::readData(std::istream &stream)
{
    stream >> m_messageType;
    stream >> m_pathName;
    if(m_messageType=="AUTH")
        m_username=m_pathName;
    if(m_messageType=="AUTH_CHALLENGE"){
        m_iv.resize(std::stoi(m_pathName));
        stream>>m_pathName;
        m_cryptoChallenge.resize(std::stoi(m_pathName));
    }
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
    if(m_messageType=="AUTH_CHALLENGE")
        std::cout<<"lunghezza iv: "+std::to_string(m_iv.size())+"\n"+"lunghezza challenge cifrata: "+std::to_string(m_cryptoChallenge.size())<<std::endl;
    if(m_messageType == "AUTH")
        std::cout<<"User che cerca di autenticarsi : "<<m_username<<std::endl;

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
        case WRONG_PASSWORD:
            responseStream << "WRONG_PASSWORD"<<"\n\n";
            break;
        case WRONG_USERNAME:
            responseStream << "WRONG_USERNAME"<<"\n\n";
            break;
        default:
            std::cout <<"nessun header"<<std::endl;
            return;

    }
    auto bufs=m_response.data();
    std::cout<<"HEADER"<<std::endl;
    std::cout<<std::string(boost::asio::buffers_begin(bufs),boost::asio::buffers_begin(bufs)+m_response.size());
    std::cout<<"FINE HEADER"<<std::endl;
    boost::asio::async_write(m_socket,
                              m_response,
                             [this,rt](boost::system::error_code ec, size_t )
                             {
                                 std::cout<<"messaggio di risposta inviato "<<std::endl;
                                 /*if(rt==CHALLENGE) {
                                     std::cout<<"aspetto la sfida cifrata"<<std::endl;
                                     async_read_until(m_socket, m_requestBuf_, "\n\n",
                                                      [this](boost::system::error_code ec, size_t bytes)
                                                      {
                                                          if (!ec)
                                                              processRead(bytes);
                                                          else
                                                              handleError(__FUNCTION__, ec);
                                                      })
                                 }*/
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
