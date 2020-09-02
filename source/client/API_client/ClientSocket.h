#pragma once

#include <vector>
#include <fstream>
#include <map>
#include <boost/asio.hpp>
#include "../../CryptoFunctions/CryptoExecutor.h"
#include <exception>
/**
 * messageType models a request type message to server
 */
enum messageType{
    UPDATE,UPDATE_NAME,REMOVE,CREATE_FILE,CREATE_DIR,SYNC_DIR,SYNC_FILE,AUTH,AUTH_CHALLENGE
};
/**
 * responsetype models a response type message from server
 */
enum responseType{
    OK,WRONG_USERNAME,WRONG_PASSWORD,CONNECTION_ERROR,UNDEFINED,CHALLENGE,INTERNAL_ERROR,NOT_PRESENT,OLD_VERSION, NON_AUTHENTICATED
};

class ClientSocket
{
public:
    using IoService = boost::asio::io_service;
    using TcpResolver = boost::asio::ip::tcp::resolver;
    using TcpResolverIterator = TcpResolver::iterator;
    using TcpSocket = boost::asio::ip::tcp::socket;

    /*+++++++++++++++++++++ Public method ++++++++++++++++++++++++++++*/
    ClientSocket(IoService& t_ioService, TcpResolverIterator t_endpointIterator);
    void update(std::string const& path);
    void updateName(std::string const& path, std::string const& newName);
    void remove(std::string const& path);
    void createFile(std::string const& path);
    void createDir(std::string const& path);
    void syncDir(std::string const& path);
    void syncFile(std::string const& path);
    void setUsername(std::string const& username);
    void setPassword(std::string const& password);

    /*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
private:
    /*+++++++++++++++++++++ Private method ++++++++++++++++++++++++++++*/
    /*###### Communication methods ######*/
    void doConnect();
    void buildHeader(messageType mt);
    void waitResponse(messageType mt);
    responseType waitChallenge();
    responseType doConnectSync();
    template<class Buffer>
    responseType writeSync(Buffer& t_buffer);
    responseType readUntilSync();
    template<class Buffer>
    void writeHeader(Buffer& t_buffer);
    template<class Buffer>
    void writeFileContent(Buffer& t_buffer);
    responseType genCryptoChallenge();
    responseType authenticate();

    /*###### Analyze response message methods ######*/
    void processResponse(size_t t_bytesTransferred,messageType mt);
    void analyzeResponse(std::string response,messageType mt);
    responseType processResponseSync();

    /*###### File manipulation methods ######*/
    void openFile(std::string const& t_path);
    void doReadFile();
    int computeDimChunk();


    /*++++++++++++++++++++++ Instance Variable +++++++++++++++++ */
    TcpResolver m_ioService;
    TcpSocket m_socket;
    TcpResolverIterator m_endpointIterator;
    std::string m_password;
    std::string m_username;
    std::vector<char> m_buf;
    boost::asio::streambuf m_request;
    std::ifstream m_sourceFile;
    std::string m_path;
    std::string m_newName;
    std::string m_mdvalue;
    int m_chunks;
    int m_sendChunks;
    std::string m_iv;
    unsigned int m_mdlen;
    messageType m_messageType;
    boost::asio::streambuf m_response;
    std::string m_responseType;
    std::string m_cryptoChallenge;
    int m_fileSize;
    int m_attempts = 0;

};
class WrongUsernameException : public std::exception{
public:
    const char* what () const throw () {return "Wrong username";}
};
class WrongPasswordException : public std::exception{
public:
    const char* what () const throw () {return "Wrong password";}
};
enum logType{
    ERROR,TRACE
};
void log(logType lt,std::string const& message);
void log(logType lt,std::string const& message1,std::vector<unsigned char> const& message2);
void log(logType lt,std::string const& message1,std::string const& message2);
void log(logType lt,std::string const& message,boost::asio::streambuf const& s);
void drawVectUnsChar(std::vector<unsigned char> const& v);
void drawStrToUnsChar(std::string const& s);
void drawHeader(boost::asio::streambuf const& s);
std::string vectUnsCharToStr(std::vector<unsigned char> const& v);
responseType stringToEnum(std::string const& s);