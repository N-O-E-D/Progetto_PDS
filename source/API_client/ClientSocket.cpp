#include <string>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <boost/log/trivial.hpp>
#include "ClientSocket.h"
//la lunghezza della challenge è definita ed è pari a LENGTHCHALLENGE
#define LENGTHCHALLENGE 100
#define DEBUG 1
/**
 * ClientSocket constructor
 * @param t_ioService
 * @param t_endpointIterator
 */
ClientSocket::ClientSocket(IoService& t_ioService, TcpResolverIterator t_endpointIterator):
                            m_ioService(t_ioService),
                            m_socket(t_ioService),
                            m_endpointIterator(t_endpointIterator)
                            {}
/**
 * ClientSocket's method which performs the authentication procedure
 * @param username
 * @param password
 * @return responseType object
 */
responseType ClientSocket::authenticate(std:: string const& username, std::string const& password){
    m_password=password;
    m_username=username;
    m_messageType=AUTH;
    buildHeader(AUTH);
    responseType rt;

    log(TRACE,"Start connection with server...");

    rt=doConnectSync();
    if(rt==CONNECTION_ERROR)
        return rt;

    log(TRACE,"Connessione accettata.\nSto inviando l'header");

    rt=writeSync(m_request);
    if(rt==CONNECTION_ERROR)
        return rt;

    log(TRACE,"Header inviato.\nAspetto la sfida");

    rt=waitChallenge();
    if(rt==CONNECTION_ERROR || rt==WRONG_USERNAME)
        return rt;

    log(TRACE,"Sfida ricevuta.\nGenero la sfida cifrata e la invio...");

    rt=genCryptoChallenge();
    if(rt==CONNECTION_ERROR)
        return rt;

    log(TRACE,"Sfida cifrata inviata.\nIn attesa dell'esito dell'autenticazione...");

    rt=waitResponseSync();
    if(rt==CONNECTION_ERROR)
        return rt;
    return processResponseSync();

}
/**
 * ClientSocket method's which performs synchronous (blocking) writing
 * @tparam Buffer
 * @param t_buffer
 * @return responseType object
 */
template<class Buffer>
responseType ClientSocket::writeSync(Buffer& t_buffer){
    boost::system::error_code error;
    boost::asio::write(m_socket,t_buffer,error);
    if(error) {
        log(ERROR,"Connection error during writeSync");
        return CONNECTION_ERROR;
    }
    return OK;
}
/**
 * ClientSocket method's which performs synchronous (blocking) reading (until)
 * @tparam Buffer
 * @param t_buffer
 * @return responseType object
 */
template<class Buffer>
responseType ClientSocket::readUntilSync(){
    boost::system::error_code error;
    responseType rt;
    m_response.consume(m_response.size());
    read_until(m_socket, m_response, "\n\n",error);
    if (error) {
        std::cout << "Connection error during waitChallenge" << std::endl;
        return CONNECTION_ERROR;
    }
    else return OK;
}
/**
 * ClientSocket method's which performs synchronous (blocking) connection
 * @return responseType object
 */
responseType ClientSocket::doConnectSync() {
    boost::system::error_code error;
    boost::asio::connect(m_socket, m_endpointIterator, error);
    if (error) {
        log(ERROR,"Connection error during doConnectSync");
        return CONNECTION_ERROR;
    }
    return OK;
}
/**
 * ClientSocket method's which performs synchronous (blocking) challenge waiting
 * @return responseType object
 */
responseType ClientSocket::waitChallenge() {
    boost::system::error_code error;
    responseType rt;
    m_response.consume(m_response.size());
    read_until(m_socket, m_response, "\n\n",error);
    if (error) {
        std::cout << "Connection error during waitChallenge" << std::endl;
        return CONNECTION_ERROR;
    }
    rt=processResponseSync();
    if(rt==WRONG_USERNAME) {
        std::cout<<"Wrong username!"<<std::endl;
        return rt;
    }
    return OK;
}
responseType ClientSocket::genCryptoChallenge(){
    std::string message(m_buf.begin(),m_buf.end());
    //debug
    printf("La challenge ricevuta è:\n");
    for (int i=0;i<message.size();i++)
        printf("%02x",(unsigned char)message[i]);
    printf("\n");
    //fine debug
    std::vector<unsigned char>iv=genRandomBytes(16);
    std::vector<unsigned char> key= HKDF (m_password,iv);
    std::vector<unsigned char> cipherChallenge = encrypt(message,iv,key);
    m_cryptoChallenge.resize(cipherChallenge.size());
    m_iv.resize(iv.size());
    for(int i=0;i<cipherChallenge.size();i++)
        m_cryptoChallenge[i]=(char) cipherChallenge[i];
    for(int i=0;i<iv.size();i++)
        m_iv[i]=(char) iv[i];
    m_messageType=AUTH_CHALLENGE;
    buildHeader(AUTH_CHALLENGE);
    writeHeader(m_request);
    std::vector<unsigned char> ss;
    ss.insert(ss.end(),iv.begin(),iv.end());
    ss.insert(ss.end(),cipherChallenge.begin(),cipherChallenge.end());
    //debug
    for (int i=0;i<iv.size()+cipherChallenge.size();i++)
        printf("%02x",ss[i]);
    printf("\n");
    //fine debug
    auto buf = boost::asio::buffer(ss.data(), iv.size()+cipherChallenge.size());
    return writeSync(buf);
}
responseType ClientSocket::waitResponseSync(){
    boost::system::error_code error;
    m_response.consume(m_response.size());
    read_until(m_socket, m_response, "\n\n",error);
    if (error) {
        std::cout << "Connection error during doConnectSync" << std::endl;
        return CONNECTION_ERROR;
    }
    return OK;
}
responseType ClientSocket::processResponseSync(){
    //debug
    auto bufs=m_response.data();
    std::cout<<"dimensione bufRequest : "<<m_response.size()<<std::endl;
    std::cout<<"HEADER"<<std::endl;
    std::cout<<std::string(boost::asio::buffers_begin(bufs),boost::asio::buffers_begin(bufs)+m_response.size());
    std::cout<<"FINE HEADER"<<std::endl;
    //fine debug
    std::istream responseStream(&m_response);
    responseStream >> m_responseType;
    std::cout<<"Ho letto :"<<m_responseType<<std::endl;
    if(m_responseType=="OK") {
        std::cout << "Ok" << std::endl;
        return OK;
    }
    if(m_responseType=="WRONG_USERNAME") {
        std::cout << "Username errato!" << std::endl;
        return WRONG_USERNAME;
    }
    if(m_responseType=="WRONG_PASSWORD") {
        std::cout << "Password errata!" << std::endl;
        return WRONG_PASSWORD;
    }
    if(m_responseType=="CHALLENGE"){
        std::cout<<"Username corretto . Leggo la sfida"<<std::endl;
        m_buf.clear();
        m_buf.resize(1);
        responseStream.read(m_buf.data(),1);
        m_buf.clear();
        m_buf.resize(LENGTHCHALLENGE);
        responseStream.read(m_buf.data(),LENGTHCHALLENGE);
        return OK;
    }
}
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
        /*case REMOVE_DIR:
            requestStream << "REMOVE_DIR\n" << m_path << "\n" << "\n\n";
            break;*/
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
        case AUTH:
            requestStream << "AUTH\n" << m_username<<"\n\n";
            break;
        case AUTH_CHALLENGE:
            requestStream <<"AUTH_CHALLENGE\n" <<std::to_string(m_iv.size())<<"\n"<< std::to_string(m_cryptoChallenge.size()) <<"\n\n";
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
    BOOST_LOG_TRIVIAL(trace) << "Request header size: " << m_request.size();
    }

void ClientSocket::update(const std::string &path,const std::function<void (std::string)> &action) {
    m_path=path;
    m_messageType=UPDATE;
    openFile(m_path);
    buildHeader(UPDATE);
    doConnect();
    waitResponse(UPDATE,action);
}
void ClientSocket::updateName(const std::string &path,std::string const& newName,const std::function<void (std::string)> &action) {
    m_path=path;
    m_messageType=UPDATE_NAME;
    m_newName=newName;
    buildHeader(UPDATE_NAME);
    doConnect();
    waitResponse(UPDATE_NAME,action);
}
void ClientSocket::remove(const std::string &path,const std::function<void (std::string)> &action) {
    m_path=path;
    m_messageType=REMOVE;
    buildHeader(REMOVE);
    doConnect();
    waitResponse(REMOVE,action);
}
/*void ClientSocket::removeDir(const std::string &path) {
    m_path=path;
    m_messageType=REMOVE_DIR;
    buildHeader(REMOVE_DIR);
    doConnect();
    waitResponse(REMOVE_DIR);
}*/
void ClientSocket::createFile(const std::string &path,const std::function<void (std::string)> &action) {
    m_path=path;
    m_messageType=CREATE_FILE;
    openFile(m_path);
    buildHeader(CREATE_FILE);
    doConnect();
    waitResponse(CREATE_FILE,action);
}
void ClientSocket::createDir(const std::string &path,const std::function<void (std::string)> &action) {
    m_path=path;
    m_messageType=CREATE_DIR;
    buildHeader(CREATE_DIR);
    doConnect();
    waitResponse(CREATE_DIR,action);
}
void ClientSocket::syncDir(std::string const& path,const std::function<void (std::string)> &action){
    m_path=path;
    m_messageType=SYNC_DIR;
    buildHeader(SYNC_DIR);
    doConnect();
    waitResponse(SYNC_DIR,action);
}
void ClientSocket::syncFile(std::string const& path,const std::function<void (std::string)> &action){
    m_path=path;
    m_messageType=SYNC_FILE;

    // Compute hash
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len=computeHash(path, md_value);

    m_mdlen=md_len;
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
    waitResponse(SYNC_FILE,action);
}
template<class Buffer>
void ClientSocket::writeHeader(Buffer& t_buffer) {
    boost::asio::async_write(m_socket,
                             t_buffer,
                             [this](boost::system::error_code ec, size_t) {
                                 if(!ec) {
                                     std::cout << "header inviato" << std::endl;

                                 }
                             });
}

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

void ClientSocket::waitResponse (messageType mt,const std::function<void (std::string)> &action){
    async_read_until(m_socket, m_response, "\n\n",
                     [this,mt,action](boost::system::error_code ec, size_t bytes)
                     {
                         if (!ec)
                             processResponse(bytes,mt,action);
                         else
                             std::cout<<"errore in waitResponse"<<std::endl;
                     });
}
void ClientSocket::processResponse(size_t t_bytesTransferred, messageType mt,const std::function<void (std::string)> &action){
    std::istream responseStream(&m_response);
    responseStream >> m_responseType;
    analyzeResponse(m_responseType,mt,action);
}
void ClientSocket::analyzeResponse(std::string response, messageType mt,const std::function<void (std::string)> &action){
    if (response=="OK") {
        std::cout << "Server ha risposto con OK , tutto è andato a buon fine" << std::endl;
        action(m_path);
    }
    if (response=="INTERNAL_ERROR") { //ritento
        std::cout<<"Server ha risposto con internal error. Qaulcosa è andato stroto , ritento"<<std::endl;
        switch (mt){
            case UPDATE:
                update(m_path,action);
                break;
            case UPDATE_NAME:
                updateName(m_path,m_newName,action);
                break;
            case REMOVE:
                remove(m_path,action);
                break;
            /*case REMOVE_DIR:
                removeDir(m_path);
                break;*/
            case CREATE_FILE:
                createFile(m_path,action);
                break;
            case CREATE_DIR:
                createDir(m_path,action);
                break;
            case SYNC_FILE:
                syncFile(m_path,action);
                break;
            case SYNC_DIR:
                syncDir(m_path,action);
                break;
        }
    }
    if (response=="NOT_PRESENT"){
        std::cout<<"Server ha risposto con not present"<<std::endl;
        if(mt==SYNC_FILE)
            createFile(m_path,action);
        if(mt==SYNC_DIR)
            createDir(m_path,action);
        if(mt==UPDATE)
            createFile(m_path,action);
        if(mt==UPDATE_NAME)
            createFile(m_path,action);
    }
    if (response=="OLD_VERSION") {
        std::cout<<"Server ha risposto con old version"<<std::endl;
        update(m_path,action);
    }
    if(response=="WRONG_USERNAME")
        std::cout<<"Username errato!"<<std::endl;
    if(response=="WRONG_PASSWORD")
        std::cout<<"Password errata!"<<std::endl;
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