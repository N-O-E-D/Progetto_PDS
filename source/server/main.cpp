#include <iostream>
#include <unistd.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <boost/asio.hpp>
#include <boost/asio/thread_pool.hpp>
#include <string>
#include <thread>

class Socket{
    int sockfd;

    Socket(int sockfd): sockfd(sockfd){
        std::cout<<"Socket "<<sockfd<<" created"<<std::endl;
    }

    Socket(const Socket &) = delete; //=delete impedisce al compilatore di creare un costruttore automaticamente
    Socket &operator=(const Socket &) = delete;

    friend class ServerSocket;

public:
    Socket(){
        //AF_INET = socket basato su ipv4
        //SOCK_STREAM = socket basato su comunicazione orientata al flusso
        //0 = chiede al sistema operativo di abbinare un protocollo in base ai due parametri precedenti
        // (in questo caso, molto probabilmente sarà TCP)
        sockfd = ::socket(AF_INET, SOCK_STREAM, 0);  //restituisce il file descriptor associato al socket
        if(sockfd < 0) throw std::runtime_error("Cannot create socket");
        std::cout << "Socket " << sockfd << " created" << std::endl;
    }

    ~Socket(){
        if(sockfd != 0){
            std::cout << "Socket " << sockfd << " closed" << std::endl;
            close(sockfd);
        }
    }

    Socket(Socket &&other) : sockfd(other.sockfd){
        other.sockfd = 0;
    }

    Socket &operator=(Socket &&other){
        if(sockfd != 0) close(sockfd);
        sockfd = other.sockfd;
        other.sockfd = 0;
        return *this;
    }

    ssize_t read(char *buffer, size_t len, int options){
        ssize_t res = recv(sockfd, buffer, len, options);
        if(res<0) throw std::runtime_error("Cannot receive from socket");
        return res;
    }

    ssize_t write(const char *buffer, size_t len, int options){
        ssize_t res = send(sockfd, buffer, len, options);
        if(res<0) throw std::runtime_error("Cannot write to socket");
        return res;
    }

    void connect(struct sockaddr_in *addr, unsigned int len){
        if(::connect(sockfd, reinterpret_cast<struct sockaddr*>(addr),len)!=0)
            throw std::runtime_error("Cannot connect to remote socket");
    }
};

//ServerSocket eredita privatamente da Socket -> i metodi pubblici di Socket diventano privati in ServerSocket, dopo averli ereditati
class ServerSocket: private Socket{
public:
    ServerSocket(int port){
        struct sockaddr_in sockaddrIn;
        sockaddrIn.sin_port = htons(port);
        sockaddrIn.sin_family = AF_INET;
        //sockaddrIn.sin_len = sizeof(sockaddrIn);
        sockaddrIn.sin_addr.s_addr = htonl(INADDR_ANY);
        if(::bind(sockfd, reinterpret_cast<struct sockaddr*>(&sockaddrIn),sizeof(sockaddrIn))!=0)
            throw std::runtime_error("Cannot bind port ");
        if(::listen(sockfd, 8)!=0)
            throw std::runtime_error("Cannot bind port ");
    }

    Socket accept(struct sockaddr_in* addr, unsigned int* len){
        int fd = ::accept(sockfd, reinterpret_cast<struct sockaddr*>(addr),(socklen_t*)len);
        if(fd<0) throw std::runtime_error("Cannot accept socket");
        return Socket(fd);
    }
};

ServerSocket ss1(5000);
ServerSocket ss2(5001);

void handleSocket1(){
    while(true){
        struct sockaddr_in addr;
        unsigned int len = sizeof(addr);
        std::cout<<"Waiting for incoming connection..."<<std::endl;
        Socket s = ss1.accept(&addr, &len);
        char name[16];
        if( inet_ntop(AF_INET, &addr.sin_addr, name, sizeof(name))==nullptr)
            throw std::runtime_error("Cannot convert a...");
        std::cout << "Got a connection from "<< name << ":" << ntohs(addr.sin_port) << "\n";
        char buffer[1024];
        int size = s.read(buffer, sizeof(buffer)-1, 0);
        buffer[size]=0; //mette il "tappo" finale, per permettere di riconoscere la fine del messaggio
        std::string str(buffer);
        s.write(buffer, size, 0); //rimando indietro quello che mi ha mandato il client
        std::cout<<"Received "<<str<<std::endl;
        std::cout << "Connection closed" << std::endl;
    }
    return;
}

void handleSocket2(){
    while(true){
        struct sockaddr_in addr;
        unsigned int len = sizeof(addr);
        std::cout<<"Waiting for incoming connection..."<<std::endl;
        Socket s = ss2.accept(&addr, &len);
        char name[16];
        if( inet_ntop(AF_INET, &addr.sin_addr, name, sizeof(name))==nullptr)
            throw std::runtime_error("Cannot convert a...");
        std::cout << "Got a connection from "<< name << ":" << ntohs(addr.sin_port) << "\n";
        char buffer[1024];
        int size = s.read(buffer, sizeof(buffer)-1, 0);
        buffer[size]=0; //mette il "tappo" finale, per permettere di riconoscere la fine del messaggio
        std::string str(buffer);
        s.write(buffer, size, 0); //rimando indietro quello che mi ha mandato il client
        std::cout<<"Received "<<str<<std::endl;
        std::cout << "Connection closed" << std::endl;
    }
    return;
}

int main() {

    //launch the pool with 4 threads
    boost::asio::thread_pool pool(4); //una coda di thread di dimensione 4: 4 thread alla volta possono essere eseguiti in parallelo

    //submit a function to the pool
    boost::asio::post(pool,handleSocket1);
    boost::asio::post(pool,handleSocket2);

    pool.join();

    return 0;
}
