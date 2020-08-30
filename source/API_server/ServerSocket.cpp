#include <iostream>

#include <boost/asio/read_until.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include "ServerSocket.h"
//la lunghezza della challenge è definita ed è pari a LENGTHCHALLENGE
#define LENGTHCHALLENGE 100
#define DIM_CHUNK 32000
#define DEBUG 1

/**
 * Session constructor
 * @param t_socket
 * @param server
 */
Session::Session(TcpSocket t_socket,Server server): m_socket(std::move(t_socket)),m_server(server){
    isAuthenticated = false;
    //self=shared_from_this();
}

/**
 * Session's method which reads until the delimeter in asynchronous way
 */
void Session::readAsyncUntil()
{
    self = shared_from_this();
    m_requestBuf_.consume(m_requestBuf_.size());
    async_read_until(m_socket, m_requestBuf_, "\n\n",
                     [this/*, self*/](boost::system::error_code ec, size_t bytes)
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

    self = shared_from_this();
    m_buf.clear();
    m_buf.resize(dim);
    log(TRACE,"Dimensione buffer : "+std::to_string(m_buf.size()));
    if(ft==READ_FILE)
        log(TRACE,"Sto leggendo il "+std::to_string(m_receivedChunks+1)+" chunk");
    m_socket.async_read_some(boost::asio::buffer(m_buf.data(), m_buf.size()),
                             [this/*, self*/,ft](boost::system::error_code ec, size_t bytes) {
                                 if (!ec)
                                    switch (ft){
                                     case READ_FILE:
                                         m_receivedChunks++;
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

    log(TRACE,"Bytes transferred :("+std::to_string(t_bytesTransferred)+"),in avail = "+std::to_string(m_requestBuf_.in_avail())+", size = "+std::to_string(m_requestBuf_.size())
                            +", max_size = "+std::to_string(m_requestBuf_.max_size())+".");

    log(TRACE,"Ho ricevuto questo header:",m_requestBuf_);
    std::istream requestStream(&m_requestBuf_);
    readData(requestStream);
    if(!isAuthenticated && (m_messageType!= "AUTH" && m_messageType!="AUTH_CHALLENGE")){
        log(ERROR,"UTENTE NON AUTENTICATO");
        sendToClient(NON_AUTHENTICATED);
    }

    else if (m_messageType=="UPDATE" || m_messageType=="CREATE_FILE") {
        log(TRACE,"Sto leggendo il contenuto del file...");
        sendToClient(OK);
        readAsyncSome(computeDimChunk(),READ_FILE);
    }
    else if(m_messageType=="AUTH") {
        if(m_server.checkCredenziali(m_username)==OK) {
            genChallenge();
        }
        else sendToClient(WRONG_USERNAME);
    }
    else if(m_messageType=="AUTH_CHALLENGE"){
        log(TRACE,"Sto leggendo l'iv e la challenge cifrata...");
        sendToClient(OK);
        readAsyncSome(m_iv.size()+m_cryptoChallenge.size(),DECRYPT_CRYPTO_CHALLENGE);
    }
    else manageMessage(m_messageType);
}
/**
 * ServerSocket's method which compute the dimension of the current chunck received from the client
 */
int Session::computeDimChunk(){
    if (m_receivedChunks == m_chunks-1 ) //se è l'ultimo chunk
        return m_fileSize-m_receivedChunks*DIM_CHUNK;
    return DIM_CHUNK;
}
/**
 * Session's method which parses and decrypts the cipher challenge received from client
 */
void Session::parseAndDecryptCryptoChallenge(){

    log(TRACE,"Ho ricevuto iv+challenge : ",std::string(m_buf.begin(),m_buf.end()));
    std::vector<unsigned char> iv_vect;
    std::vector<unsigned char> cc_vect;
    cc_vect.resize(m_cryptoChallenge.size());
    iv_vect.resize(m_iv.size());
    for (int i=0;i<m_iv.size();i++)
        iv_vect[i]=(unsigned char) m_buf[i];
    for (int i=m_iv.size();i<m_iv.size()+m_cryptoChallenge.size();i++)
        cc_vect[i-m_iv.size()]=(unsigned char) m_buf[i];

    log(TRACE,"Ho letto iv: ",iv_vect);
    log(TRACE,"Ho letto la sfida cifrata: ",cc_vect);
    std::string password;
    if (m_server.UserToPassword(m_username,password)==WRONG_USERNAME){
        sendToClient(WRONG_USERNAME);
        return;
    }
    std::vector<unsigned char> key=HKDF(password, iv_vect);
    log(TRACE,"Lunghezza chiave : "+std::to_string(key.size()));
    std::vector<unsigned char> dec_challenge=decrypt(cc_vect,iv_vect,key);
    if(CRYPTO_memcmp((unsigned char*)m_challenge.data(),dec_challenge.data(),LENGTHCHALLENGE)==0) {
        log(TRACE,"AUTENTICAZIONE RIUSCITA");
        isAuthenticated=true;
        sendToClient(OK);
        readAsyncUntil();
    }
    else {
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

    log(TRACE,"La challenge generata è : ",m_challenge);
    sendToClient(CHALLENGE);
    //readAsyncUntil();

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
        log(TRACE,"Lunghezza iv: "+std::to_string(m_iv.size()));
        log(TRACE,"Lunghezza challenge cifrata: "+std::to_string(m_cryptoChallenge.size()));
    }
    if(m_messageType=="UPDATE_NAME") {
        stream >> m_newName;
        log(TRACE,"Nuovo nome : "+m_newName);
    }
    if(m_messageType=="UPDATE" || m_messageType=="CREATE_FILE") {
        stream >> m_fileSize;
        log(TRACE,"Lunghezza file : "+std::to_string(m_fileSize));
        m_chunks=((int)m_fileSize)/DIM_CHUNK +1;
        m_receivedChunks=0;
        log(TRACE,"Il numero dei chunks da leggere è pari a : "+std::to_string(m_chunks));
    }
    if (m_messageType=="SYNC_FILE") {
        stream >> m_mdvalue;
        log(TRACE, "Il message digest ricevuto è : ",m_mdvalue);
    }
    if(m_messageType=="SYNC_DIR"){
        log(TRACE,"Il path della cartella da sincronizzare è :"+m_pathName);
    }

}
/**
 * Session's method which reads the file received from client
 * @param t_bytesTransferred
 */
void Session::doReadFileContent(size_t t_bytesTransferred)
{
    if (t_bytesTransferred > 0) {
        log(TRACE,"Bytes ricevuti "+std::to_string(t_bytesTransferred));
       m_file.insert(m_file.end(),m_buf.begin(),m_buf.end());
    }
    log(TRACE,"Ho ricevuto questo chunk : "+std::string(m_file.begin(),m_file.end()));
    manageMessage(m_messageType);

}
/**
 * Session's method which call the correct server method according to messageType
 * @param messageType
 */
void Session::manageMessage(std::string const& messageType){
    log(TRACE,"Richiamo metodi del server ("+messageType+")...");
    responseType rt;
    m_server.setUserDirectory(m_username);
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
    if(m_messageType=="UPDATE" || m_messageType=="CREATE_FILE")
        if(m_receivedChunks<m_chunks)
            readAsyncSome(computeDimChunk(),READ_FILE);
        else m_receivedChunks=0;

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
        case NON_AUTHENTICATED:
            responseStream << "NON_AUTHENTICATED"<<"\n\n";
            break;
        default:
            log(TRACE,"nessun header");
            return;

    }
    log(TRACE,"L'header da inviare al client è:",m_response);
    boost::asio::async_write(m_socket,
                              m_response,
                             [this,rt](boost::system::error_code ec, size_t )
                             {

                                log(TRACE,"Messaggio di risposta inviato.");
                                if(rt==OLD_VERSION || rt==NOT_PRESENT){
                                    log(TRACE,"Aspetto che il client mi invii il file...");
                                    readAsyncUntil();
                                }
                                if(rt==CHALLENGE)
                                    readAsyncUntil();
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
    log(TRACE,"Server in ascolto...");
    m_acceptor.async_accept(m_socket,
                            [this](boost::system::error_code ec)
                            {
                                if (!ec) {
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
