#include <iostream>

#include <boost/asio/read_until.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include "ServerSocket.h"
//la lunghezza della challenge è definita ed è pari a LENGTHCHALLENGE
#define LENGTHCHALLENGE 100
#define DEBUG 1

/**
 * Session constructor
 * @param t_socket
 * @param server
 */
Session::Session(TcpSocket t_socket,Server server): m_socket(std::move(t_socket)),m_server(server){}

/**
 * Session's method which reads until the delimeter in asynchronous way
 */
void Session::readAsyncUntil()
{
    auto self = shared_from_this();
    m_requestBuf_.consume(m_requestBuf_.size());
    async_read_until(m_socket, m_requestBuf_, "\n\n",
                     [this, self](boost::system::error_code ec, size_t bytes)
                     {
                        if (!ec)
                             processRead(bytes);
                         else
                             handleError(__FUNCTION__, ec);
                     });
}

/**
 * Session's method which read some ( dim ) bytes
 * @param action it is the callbak to call when the reading is completed
 * @param dim is the length ok the message to read
 */
void Session::readAsyncSome(int dim,functionType ft){

    auto self = shared_from_this();
    m_buf.clear();
    m_buf.resize(dim);
    //std::cout << "dimensione buffer : " << m_buf.size()<<std::endl;
    log(TRACE,"Dimensione buffer : "+std::to_string(m_buf.size()));

    m_socket.async_read_some(boost::asio::buffer(m_buf.data(), dim),
                             [this, self,ft](boost::system::error_code ec, size_t bytes) {
                                 if (!ec)
                                    switch (ft){
                                     case READ_FILE:
                                         doReadFileContent(bytes);
                                         break;
                                     case DECRYPT_CRYPTO_CHALLENGE:
                                         parseAndDecryptCryptoChallenge();
                                         break;
                                 }
                                 else log(ERROR,"Errore nella async read some : "+ec.message());
                             });
}
/**
 * Session's method which processes the message from client
 * @param t_bytesTransferred
 */
void Session::processRead(size_t t_bytesTransferred)
{
    /*BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << "(" << t_bytesTransferred << ")"
                             << ", in_avail = " << m_requestBuf_.in_avail() << ", size = "
                             << m_requestBuf_.size() << ", max_size = " << m_requestBuf_.max_size() << ".";*/
    //debug
    log(TRACE,"Bytes transferred :("+std::to_string(t_bytesTransferred)+"),in avail = "+std::to_string(m_requestBuf_.in_avail())+", size = "+std::to_string(m_requestBuf_.size())
                            +", max_size = "+std::to_string(m_requestBuf_.max_size())+".");

    /*auto bufs=m_requestBuf_.data();
    std::cout<<"dimensione bufRequest : "<<m_requestBuf_.size()<<std::endl;
    std::cout<<"HEADER"<<std::endl;
    std::cout<<std::string(boost::asio::buffers_begin(bufs),boost::asio::buffers_begin(bufs)+m_requestBuf_.size());
    std::cout<<"FINE HEADER"<<std::endl;
    //fine debug*/
    log(TRACE,"Ho ricevuto questo header:",m_requestBuf_);
    std::istream requestStream(&m_requestBuf_);
    readData(requestStream);
    //m_requestBuf_.consume(m_requestBuf_.size());
    if (m_messageType=="UPDATE" || m_messageType=="CREATE_FILE") {
        //std::cout<<"Sto leggendo il contenuto del file..."<<std::endl;
        log(TRACE,"Sto leggendo il contenuto del file...");
        readAsyncSome(m_fileSize,READ_FILE);
        /*auto self = shared_from_this();
        m_buf.resize(m_fileSize);
        std::cout << "dimensione buffer : " << m_buf.size()<<std::endl;
        m_socket.async_read_some(boost::asio::buffer(m_buf.data(), m_fileSize),
                                 [this, self](boost::system::error_code ec, size_t bytes) {
                                     if (!ec)
                                         doReadFileContent(bytes);
                                 });*/
    }
    else if(m_messageType=="AUTH") {
        if(m_server.checkCredenziali(m_username)==OK) {
            //sendToClient(OK);
            genChallenge();
        }
        else sendToClient(WRONG_USERNAME);
    }
    else if(m_messageType=="AUTH_CHALLENGE"){
        //std::cout<<"Sto leggendo l'iv e la challenge cifrata..."<<std::endl;
        log(TRACE,"Sto leggendo l'iv e la challenge cifrata...");
        readAsyncSome(m_iv.size()+m_cryptoChallenge.size(),DECRYPT_CRYPTO_CHALLENGE);
        /*auto self = shared_from_this();
        m_buf.clear();
        m_buf.resize(m_iv.size()+m_cryptoChallenge.size());
        std::cout<<m_buf.size()<<std::endl;
        m_socket.async_read_some(boost::asio::buffer(m_buf.data(), m_buf.size()),
                                 [this, self](boost::system::error_code ec, size_t bytes) {
                                     if (!ec)
                                         parseAndDecryptCryptoChallenge();
                                     else std::cout<<"Errore nella lettura dell'iv e della challenge cifrata : "<<ec.message()<<std::endl;
                                 });*/
    }
    else manageMessage(m_messageType);
}
/**
 * Session's method which parses and decrypts the cipher challenge received from client
 */
void Session::parseAndDecryptCryptoChallenge(){
    //debug
    /*printf("ho ricevuto: \n");
    for (int i=0;i<m_iv.size()+m_cryptoChallenge.size();i++)
        printf("%02x",(unsigned char)m_buf[i]);
    printf("\n");*/
    //fine debug
    log(TRACE,"Ho ricevuto iv+challenge : ",std::string(m_buf.begin(),m_buf.end()));
    std::vector<unsigned char> iv_vect;
    std::vector<unsigned char> cc_vect;
    cc_vect.resize(m_cryptoChallenge.size());
    iv_vect.resize(m_iv.size());
    for (int i=0;i<m_iv.size();i++)
        iv_vect[i]=(unsigned char) m_buf[i];
    for (int i=m_iv.size();i<m_iv.size()+m_cryptoChallenge.size();i++)
        cc_vect[i-m_iv.size()]=(unsigned char) m_buf[i];
    //debug
    /*printf("Ho letto iv: \n");
    for (int i=0;i<m_iv.size();i++)
        printf("%02x",iv_vect[i]);
    printf("\n");
    printf("Ho letto la sfida cifrata: \n");
    for (int i=0;i<cc_vect.size();i++)
        printf("%02x",cc_vect[i]);
    printf("\n");*/
    //fine debug
    log(TRACE,"Ho letto iv: ",iv_vect);
    log(TRACE,"Ho letto la sfida cifrata: ",cc_vect);
    std::string password;
    if (m_server.UserToPassword(m_username,password)==WRONG_USERNAME){
        sendToClient(WRONG_USERNAME);
        return;
    }
    std::vector<unsigned char> key=HKDF(password, iv_vect);
    //std::cout<<"Lunghezza chiave : "<<key.size()<<std::endl;
    log(TRACE,"Lunghezza chiave : "+std::to_string(key.size()));
    std::vector<unsigned char> dec_challenge=decrypt(cc_vect,iv_vect,key);
    if(CRYPTO_memcmp((unsigned char*)m_challenge.data(),dec_challenge.data(),LENGTHCHALLENGE)==0) {
        //std::cout << "AUTENTICAZIONE RIUSCITA" << std::endl;
        log(TRACE,"AUTENTICAZIONE RIUSCITA");
        sendToClient(OK);
    }
    else {
        //std::cout<<"AUTENTICAZIONE FALLITA"<<std::endl;
        log(TRACE,"AUTENTICAZIONE FALLITA");
        sendToClient(WRONG_PASSWORD);
    }
}
/**
 * Session's method which generates the challange and sends it to client
 */
void Session::genChallenge(){
    std::vector<unsigned char> challenge = genRandomBytes(LENGTHCHALLENGE);
    m_challenge.resize(challenge.size());
    for (int i=0;i<challenge.size();i++)
        m_challenge[i]=(char)challenge[i];
    //debug
    /*printf("La challenge generata è: \n");
    for (int i=0;i<m_challenge.size();i++)
        printf("%02x",(unsigned char)m_challenge[i]);
    printf("\n");
    std::cout<<challenge.size()<<std::endl;*/
    //fine debug
    log(TRACE,"La challenge generata è : ",m_challenge);
    sendToClient(CHALLENGE);
    readAsyncUntil();
    //auto self=shared_from_this();
    //auto buf = boost::asio::buffer(challenge.data(), LENGTHCHALLENGE);
    //printf("eeee\n");
    /*boost::asio::async_write(m_socket,
                             buf,
                             [this](boost::system::error_code ec, size_t bytes)
                             {
                                std::cout<<"challenge inviata, aspetto la challenge cifrata..."<<std::endl;
                                if(!ec){
                                    m_requestBuf_.consume(m_requestBuf_.size());
                                    async_read_until(m_socket, m_requestBuf_, "\n\n",
                                                     [this](boost::system::error_code ec, size_t bytes)
                                                     {

                                                         if (!ec)
                                                             processRead(bytes);
                                                         else
                                                             handleError(__FUNCTION__, ec);
                                                     });
                                }


                             });*/
}
/**
 * Session's method which parses the data (from stream) receved from client
 * @param stream
 */
void Session::readData(std::istream &stream)
{
    stream >> m_messageType;
    stream >> m_pathName;
    log(TRACE,"Message type ricevuto : "+ m_messageType);
    if(m_messageType=="AUTH"){
        m_username=m_pathName;
        log(TRACE,"User che cerca di autenticarsi : "+ m_username);
    }
    if(m_messageType=="AUTH_CHALLENGE"){
        m_iv.resize(std::stoi(m_pathName));
        stream>>m_pathName;
        m_cryptoChallenge.resize(std::stoi(m_pathName));
        log(TRACE,"Lunghezza iv: "+std::to_string(m_iv.size())+"\n"+"Lunghezza challenge cifrata: "+std::to_string(m_cryptoChallenge.size()));
    }
    if(m_messageType=="UPDATE_NAME") {
        stream >> m_newName;
        log(TRACE,"Nuovo nome : "+m_newName);
    }
    if(m_messageType=="UPDATE" || m_messageType=="CREATE_FILE") {
        stream >> m_fileSize;
        log(TRACE,"Lunghezza file : "+m_fileSize);
    }
    if (m_messageType=="SYNC_FILE") {
        stream >> m_mdvalue;
        log(TRACE, "Il message digest ricevuto è : ",m_mdvalue);
    }
    if(m_messageType=="SYNC_DIR"){
        log(TRACE,"Il path della cartella da sincronizzare è :"+m_pathName);
    }
        
    //debug
    /*std::cout<< m_messageType<<std::endl;
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
                             << ", tellg = " << stream.tellg();*/

}
/**
 * Session's method which reads the file received from client
 * @param t_bytesTransferred
 */
void Session::doReadFileContent(size_t t_bytesTransferred)
{
    if (t_bytesTransferred > 0) {
       m_file.insert(m_file.end(),m_buf.begin(),m_buf.end());
    }
    //std::cout<<"ho ricevuto questo file: "<<std::endl;
    /*for (int i=0;i<m_file.size();i++)
        std::cout<<m_file.at(i);*/
    log(TRACE,"Ho ricevuto questo file : "+std::string(m_file.begin(),m_file.end()));
    manageMessage(m_messageType);

}
/**
 * Session's method which call the correct server method according to messageType
 * @param messageType
 */
void Session::manageMessage(std::string const& messageType){
    //std::cout<<"Invio al server "<<messageType<<std::endl;
    log(TRACE,"Richiamo metodi del server ("+messageType+")...");
    responseType rt;
    //ora che ho tutti i dati ricevuti dal client richiamo le funzioni fornite dalla classe Server a seconda dell'azione
    if (m_messageType=="UPDATE")
        rt=m_server.update(m_pathName,m_file,m_fileSize);
    if (m_messageType=="UPDATE_NAME")
        rt=m_server.updateName(m_pathName,m_newName);
    if (m_messageType=="REMOVE")
        rt=m_server.removeDir(m_pathName);
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
/**
 * Session method's which builds the header and sends it to client
 * @param rt
 */
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
        case CHALLENGE:
            responseStream << "CHALLENGE\n"<<m_challenge<<"\n\n";
            break;
        default:
            std::cout <<"nessun header"<<std::endl;
            return;

    }
    //debug
    /*auto bufs=m_response.data();
    std::cout<<"HEADER"<<std::endl;
    std::cout<<std::string(boost::asio::buffers_begin(bufs),boost::asio::buffers_begin(bufs)+m_response.size());
    std::cout<<"FINE HEADER"<<std::endl;*/
    //fine debug
    log(TRACE,"L'header da inviare al client è:",m_response);
    boost::asio::async_write(m_socket,
                              m_response,
                             [this,rt](boost::system::error_code ec, size_t )
                             {
                                log(TRACE,"Messaggio di risposta inviato.");
                                //std::cout<<"messaggio di risposta inviato "<<std::endl;
                             });
}
/**
 * Session's method which handles an error
 * @param t_functionName
 * @param t_ec
 */
void Session::handleError(std::string const& t_functionName, boost::system::error_code const& t_ec)
{
    BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << " in " << t_functionName << " due to "
                             << t_ec << " " << t_ec.message() << std::endl;
}

/**
 * ServerSocket constructor
 * @param t_ioService
 * @param t_port
 * @param server
 */
ServerSocket::ServerSocket(IoService& t_ioService, short t_port, Server& server)
        : m_socket(t_ioService),
          m_acceptor(t_ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), t_port)),
          m_server(server)
{
    log(TRACE,"Server started...");

    doAccept();
}

/**
 * ServerSocket's method which accetps incoming request from a client
 */
void ServerSocket::doAccept()
{
    //std::cout<<"server in ascolto"<<std::endl;
    log(TRACE,"Server in ascolto...");
    m_acceptor.async_accept(m_socket,
                            [this](boost::system::error_code ec)
                            {
                                if (!ec) {
                                    //std::cout<<"Comunicazione accettata\n";
                                    log(TRACE,"Comunicazione accettata.");
                                    std::make_shared<Session>(std::move(m_socket), std::move(m_server))->start();
                                }

                                doAccept();
                            });
}

void log(logType lt,std::string const& message){
#if DEBUG
    switch(lt){
        case ERROR:
            BOOST_LOG_TRIVIAL(error) << message;
            break;
        case TRACE:
            BOOST_LOG_TRIVIAL(trace) << message;
            break;
    }
#endif
}
void log(logType lt,std::string const& message, boost::asio::streambuf const& s){
#if DEBUG
    switch(lt){
        case ERROR:
            BOOST_LOG_TRIVIAL(error) << message;
            drawHeader(s);
            break;
        case TRACE:
            BOOST_LOG_TRIVIAL(trace) << message;
            drawHeader(s);
            break;
    }
#endif
}
void log(logType lt,std::string const& message1,std::string const& message2){
#if DEBUG
    switch(lt){
        case ERROR:
            BOOST_LOG_TRIVIAL(error) << message1;
            drawStrToUnsChar(message2);
            break;
        case TRACE:
            BOOST_LOG_TRIVIAL(trace) << message1;
            drawStrToUnsChar(message2);
            break;
    }
#endif
}
void log(logType lt,std::string const& message1,std::vector<unsigned char> const& message2){
#if DEBUG
    switch(lt){
        case ERROR:
            BOOST_LOG_TRIVIAL(error) << message1;
            drawVectUnsChar(message2);
            break;
        case TRACE:
            BOOST_LOG_TRIVIAL(trace) << message1;
            drawVectUnsChar(message2);
            break;
    }
#endif
}
void drawStrToUnsChar(std::string const& s){
    for (int i=0;i<s.size();i++)
        printf("%02x",(unsigned char)s[i]);
    printf("\n");
}
void drawHeader(boost::asio::streambuf const& s){
    auto bufs=s.data();
    std::cout<<"dimensione header : "<<s.size()<<std::endl;
    std::cout<<"HEADER"<<std::endl;
    std::cout<<std::string(boost::asio::buffers_begin(bufs),boost::asio::buffers_begin(bufs)+s.size());
    std::cout<<"FINE HEADER"<<std::endl;
}
void drawVectUnsChar(std::vector<unsigned char> const& v){
    for (int i=0;i<v.size();i++)
        printf("%02x",v[i]);
    printf("\n");
}