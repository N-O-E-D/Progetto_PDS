#include <string>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <boost/log/trivial.hpp>
#include "ClientSocket.h"
//la lunghezza della challenge è definita ed è pari a LENGTHCHALLENGE
#define LENGTHCHALLENGE 100
#define DIM_CHUNK 32000
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
 * ClientSocket's method which performs the authentication procedure (in synchronous or blocking way)
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

    rt=readUntilSync();
    if(rt==CONNECTION_ERROR)
        return rt;
    return processResponseSync();

}
/**
 * ClientSocket's method which performs synchronous (blocking) writing
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
 * ClientSocket's method which performs synchronous (blocking) reading (until)
 * @tparam Buffer
 * @param t_buffer
 * @return responseType object
 */
responseType ClientSocket::readUntilSync(){
    boost::system::error_code error;
    m_response.consume(m_response.size());
    read_until(m_socket, m_response, "\n\n",error);
    if (error) {
        log(ERROR,"Connection error during waitChallenge");
        return CONNECTION_ERROR;
    }
    else return OK;
}
/**
 * ClientSocket's method which performs synchronous (blocking) connection
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
 * ClientSocket's method which performs synchronous (blocking) challenge waiting
 * @return responseType object
 */
responseType ClientSocket::waitChallenge() {
    responseType rt;
    rt=readUntilSync();
    if(rt==CONNECTION_ERROR)
        return rt;
    rt=processResponseSync();
    if(rt==WRONG_USERNAME) {
        log(ERROR,"Wrong username!");
        return rt;
    }
    return OK;
}
/**
 * ClientSocket's method which generate the cipher challenge and send it to server
 * @return responseType
 */
responseType ClientSocket::genCryptoChallenge(){

    std::string message(m_buf.begin(),m_buf.end());
    log(TRACE,"La challenge ricevuta è:",message);
    // iv generation
    std::vector<unsigned char> iv=genRandomBytes(16);
    // hkdf generation starting from password inserted by user (it is more secure!)
    std::vector<unsigned char> key= HKDF (m_password,iv);
    // challenge ciphering
    std::vector<unsigned char> cipherChallenge = encrypt(message,iv,key);
    //vector<unsigned char> translation to string for correct delivery to server
    m_cryptoChallenge=vectUnsCharToStr(cipherChallenge);
    m_iv=vectUnsCharToStr(iv);

    m_messageType=AUTH_CHALLENGE;
    buildHeader(AUTH_CHALLENGE);
    writeHeader(m_request);

    std::vector<unsigned char> ss;
    ss.insert(ss.end(),iv.begin(),iv.end());
    ss.insert(ss.end(),cipherChallenge.begin(),cipherChallenge.end());
    log(TRACE,"invio: ",ss);
    auto buf = boost::asio::buffer(ss.data(), m_iv.size()+m_cryptoChallenge.size());
    return writeSync(buf);
}
/**
 * ClientSocket's method which process the server response in synchronous (blocking) way
 * @return responseType object
 */
responseType ClientSocket::processResponseSync(){
    log(TRACE,"Ho ricevuto questo header:",m_response);
    std::istream responseStream(&m_response);
    responseStream >> m_responseType;
    log(TRACE,"Ho letto : "+m_responseType);
    responseType rt= stringToEnum(m_responseType);
    switch(rt){
        case OK:
            log(TRACE,"OK autenticato");
            return OK;
        case WRONG_USERNAME:
            log(TRACE,"Username errato!");
            return WRONG_USERNAME;
        case WRONG_PASSWORD:
            log(TRACE,"Password errata!");
            return WRONG_PASSWORD;
        case CHALLENGE:
            log(TRACE,"Username corretto. Leggo la sfida...");
            m_buf.clear();
            m_buf.resize(1);
            responseStream.read(m_buf.data(),1);
            m_buf.clear();
            m_buf.resize(LENGTHCHALLENGE);
            responseStream.read(m_buf.data(),LENGTHCHALLENGE);
            return OK;
        case UNDEFINED :
            log(TRACE,"Undefined header!");
            return CONNECTION_ERROR;
    }
}
/**
 * ClientSocket's method which open a file
 * @param t_path
 */
void ClientSocket::openFile(std::string const& t_path)
{
    m_sourceFile.open(t_path, std::ios_base::binary);
    if (m_sourceFile.fail())
        throw std::fstream::failure("Failed while opening file " + t_path);
    else log(TRACE,t_path+" aperto");
    m_sourceFile.seekg(0, m_sourceFile.end);
    m_fileSize = m_sourceFile.tellg();
    m_sourceFile.seekg(0, m_sourceFile.beg);
    m_chunks = m_fileSize/DIM_CHUNK + 1;
    m_sendChunks=0;
    log(TRACE,"Il numero di chuncks è : "+std::to_string(m_chunks));

}
/**
 * ClientSocket's method which starts an asynchronous (non-blocking) connection
 */
void ClientSocket::doConnect()
{
    log(TRACE,"Start connection with server...");
    boost::asio::async_connect(m_socket, m_endpointIterator,
                               [this](boost::system::error_code ec, TcpResolverIterator)
                               {
                                   if (!ec) {
                                       //invio l'header
                                       log(TRACE,"Connesso. Invio header");
                                       writeHeader(m_request);
                                       //controllo anche se devo inviare il contenuto di un file
                                       if(m_messageType==UPDATE || m_messageType==CREATE_FILE)
                                           doReadFile();

                                   } else log(ERROR,"Couldn't connect to host. Please run server "
                                                    "or check network connection : "+ec.message());
                               });

}
/**
 * ClientSocket's method which read a file content and then send the file to server
 * @param t_ec
 */
void ClientSocket::doReadFile()
{

        if (m_sourceFile) {
            m_buf.resize(computeDimChunk());
            m_sourceFile.read(m_buf.data(), m_buf.size());
            if (m_sourceFile.fail() && !m_sourceFile.eof()) {
                log(ERROR,"Failed while reading file");
                throw std::fstream::failure("Failed while reading file");
            }
            
            //log(TRACE,"Send"+std::to_string(m_sourceFile.gcount())+ " bytes, total: "+std::to_string(m_sourceFile.tellg())+" bytes");
            //log(TRACE,"Il contenuto del chunk da inviare è :",std::string(m_buf.begin(),m_buf.end()));
            auto buf = boost::asio::buffer(m_buf.data(), static_cast<size_t>(m_sourceFile.gcount()));
            log(TRACE,"Dimensione buffer : "+std::to_string(buf.size()));
            //m_sourceFile.close();
            writeFileContent(buf);
        }

}
/**
 * ClientSocket's method which compute the dimension of the current chunck to send to server
 */
int ClientSocket::computeDimChunk(){
    if (m_sendChunks == m_chunks-1 ) //se è l'ultimo chunk
        return m_fileSize-m_sendChunks*DIM_CHUNK;
    return DIM_CHUNK;
}
/**
 * ClientSocket's method which builds the correct header accord to messageType passed as parameter
 * @param mt
 */
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
    log(TRACE,"L'header costruito è:",m_request);

    }
/**
 * ClientSocket's method which performs an update action (send to server the path and then the new content of the file)
 * @param path
 * @param action
 */
void ClientSocket::update(const std::string &path,const std::function<void (std::string)> &action) {
    m_path=path;
    m_messageType=UPDATE;
    openFile(m_path);
    buildHeader(UPDATE);
    doConnect();
    waitResponse(UPDATE,action);
}
/**
 * ClientSocket's method which performs an update name action (send to server the path and then the new name of the file)
 * @param path
 * @param newName
 * @param action
 */
void ClientSocket::updateName(const std::string &path,std::string const& newName,const std::function<void (std::string)> &action) {
    m_path=path;
    m_messageType=UPDATE_NAME;
    m_newName=newName;
    buildHeader(UPDATE_NAME);
    doConnect();
    waitResponse(UPDATE_NAME,action);
}
/**
 * ClientSocket's method which performs a remove action (send to server the path to remove)
 * @param path
 * @param action
 */
void ClientSocket::remove(const std::string &path,const std::function<void (std::string)> &action) {
    m_path=path;
    m_messageType=REMOVE;
    buildHeader(REMOVE);
    doConnect();
    waitResponse(REMOVE,action);
}
/**
 * ClientSocket's method which performs a create file action (send to server the path and then the content of the file)
 * @param path
 * @param action
 */
void ClientSocket::createFile(const std::string &path,const std::function<void (std::string)> &action) {
    m_path=path;
    m_messageType=CREATE_FILE;
    openFile(m_path);
    buildHeader(CREATE_FILE);
    doConnect();
    waitResponse(CREATE_FILE,action);
}
/**
 * ClientSocket's method which performs a create directory action (send to server the path)
 * @param path
 * @param action
 */
void ClientSocket::createDir(const std::string &path,const std::function<void (std::string)> &action) {
    m_path=path;
    m_messageType=CREATE_DIR;
    buildHeader(CREATE_DIR);
    doConnect();
    waitResponse(CREATE_DIR,action);
}
/**
 * ClientSocket's method which performs a syncronization directory action (send to server the path)
 * @param path
 * @param action
 */
void ClientSocket::syncDir(std::string const& path,const std::function<void (std::string)> &action){
    m_path=path;
    m_messageType=SYNC_DIR;
    buildHeader(SYNC_DIR);
    doConnect();
    waitResponse(SYNC_DIR,action);
}
/**
 * ClientSocket's method which performs a syncronization file action (send to server the path and then the hash of the file)
 * @param path
 * @param action
 */
void ClientSocket::syncFile(std::string const& path,const std::function<void (std::string)> &action){
    m_path=path;
    m_messageType=SYNC_FILE;

    // Compute hash
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len=computeHash(path, md_value);

    m_mdlen=md_len;
    /*printf("digest: ");
    for(int i = 0; i < md_len; i++)
        printf("%02x", md_value[i]);
    printf("\n");*/
    log(TRACE,"The digest is:",std::vector<unsigned char>(md_value,md_value+md_len));
    std::string sName(reinterpret_cast<char* >(md_value),(size_t) md_len);
    m_mdvalue=sName;
    /*for(int i = 0; i < md_len; i++)
        printf("%02x", (unsigned char)sName[i]);*/
    log(TRACE,"The digest (string) is:",m_mdvalue);
    buildHeader(SYNC_FILE);
    doConnect();
    waitResponse(SYNC_FILE,action);
}
/**
 * ClientSocket's method which sends to server the header in asynchronous way
 * @tparam Buffer
 * @param t_buffer
 */
template<class Buffer>
void ClientSocket::writeHeader(Buffer& t_buffer) {
    boost::asio::async_write(m_socket,
                             t_buffer,
                             [this](boost::system::error_code ec, size_t) {
                                 if(!ec) {
                                     log(TRACE,"Header inviato");

                                 }
                             });
}
/**
 * ClientSocket's method which sends to server the content of file in asynchronous way
 * @tparam Buffer
 * @param t_buffer
 */
template<class Buffer>
void ClientSocket::writeFileContent(Buffer& t_buffer)
{
    boost::asio::async_write(m_socket,
                             t_buffer,
                             [this](boost::system::error_code ec, size_t )
                             {
                                 log(TRACE,"Chunck inviato");
                                 m_sendChunks++;

                             });
    };
/**
 * ClientSocket's method which read the response from server in asynchronous way
 * @param mt
 * @param action
 */
void ClientSocket::waitResponse (messageType mt,const std::function<void (std::string)> &action){
    m_response.consume(m_response.size());
    async_read_until(m_socket, m_response, "\n\n",
                     [this,mt,action](boost::system::error_code ec, size_t bytes)
                     {
                         if (!ec)
                             processResponse(bytes,mt,action);
                         else
                             log(ERROR,"errore in waitResponse");
                     });
}
/**
 * ClientSocket's method which process the response from server
 * @param t_bytesTransferred
 * @param mt
 * @param action
 */
void ClientSocket::processResponse(size_t t_bytesTransferred, messageType mt,const std::function<void (std::string)> &action){
    std::istream responseStream(&m_response);
    responseStream >> m_responseType;
    analyzeResponse(m_responseType,mt,action);
}
/**
 * Clientsocket's method which analyses the response from server
 * @param response
 * @param mt
 * @param action
 */
void ClientSocket::analyzeResponse(std::string response, messageType mt,const std::function<void (std::string)> &action){
    log(TRACE,response);
    responseType rt=stringToEnum(response);
    switch(rt){
        case OK:
            log(TRACE,"Server ha risposto con OK , tutto è andato a buon fine");
            if (m_messageType==CREATE_FILE || m_messageType==UPDATE) {
                log(TRACE,"I chunks inviati sono : "+std::to_string(m_sendChunks));
                if (m_sendChunks < m_chunks) {
                    doReadFile();
                    waitResponse(mt, action);
                }
                else {
                    log(TRACE,"Intero file inviato correttamente.");
                    m_sendChunks = 0;
                    m_sourceFile.close();
                }
            }
            break;
            case INTERNAL_ERROR: //ritento
            log(TRACE,"Server ha risposto con internal error. Qualcosa è andato storto , ritento");
            switch(mt){
                case UPDATE:
                    update(m_path,action);
                    break;
                case UPDATE_NAME:
                    updateName(m_path,m_newName,action);
                    break;
                case REMOVE:
                    remove(m_path,action);
                    break;
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
            break;
        case NOT_PRESENT:
            log(TRACE,"Server ha risposto con not present");
            if(mt==SYNC_FILE)
                createFile(m_path,action);
            if(mt==SYNC_DIR)
                createDir(m_path,action);
            if(mt==UPDATE)
                createFile(m_path,action);
            if(mt==UPDATE_NAME)
                createFile(m_path,action);
            break;
        case OLD_VERSION:
            log(TRACE,"Server ha risposto con old version");
            update(m_path,action);
            break;
    }

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
void drawVectUnsChar(std::vector<unsigned char> const& v){
    for (int i=0;i<v.size();i++)
        printf("%02x",v[i]);
    printf("\n");
}
void drawStrToUnsChar(std::string const& s){
    for (int i=0;i<s.size();i++)
        printf("%02x",(unsigned char)s[i]);
    printf("\n");
}
std::string vectUnsCharToStr(std::vector<unsigned char> const& v){
    std::string result;
    result.resize(v.size());
    for(int i=0;i<v.size();i++)
        result[i]=(char) v[i];
    return result;
}
void drawHeader(boost::asio::streambuf const& s){
    auto bufs=s.data();
    std::cout<<"dimensione header : "<<s.size()<<std::endl;
    std::cout<<"HEADER"<<std::endl;
    std::cout<<std::string(boost::asio::buffers_begin(bufs),boost::asio::buffers_begin(bufs)+s.size());
    std::cout<<"FINE HEADER"<<std::endl;
}
responseType stringToEnum(std::string const& s){
    std::unordered_map <std::string,responseType> table={{"OK",OK},{"WRONG_USERNAME",WRONG_USERNAME},{"WRONG_PASSWORD",WRONG_PASSWORD},
                                                         {"CONNECTION_ERROR",CONNECTION_ERROR},{"UNDEFINED",UNDEFINED},{"CHALLENGE",CHALLENGE},
                                                         {"INTERNAL_ERROR",INTERNAL_ERROR},{"NOT_PRESENT",NOT_PRESENT},{"OLD_VERSION" , OLD_VERSION}};
    auto it=table.find(s);
    if(it != table.end())
        return it->second;
    else return UNDEFINED;
}
