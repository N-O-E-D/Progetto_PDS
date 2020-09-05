# Progetto PDS - Remote Backup
## Installing / Setting Up
- Install Boost and OpenSSL libraries
- ```
  git clone https://github.com/N-O-E-D/Progetto_PDS.git
  cd Progetto_PDS
  cd source
  ```
- Server Side
  - ```
    cd server
    cmake .
    cmake --build .
    ```
- Client Side
  - ```
    cd client
    cmake .
    cmake --build .
    ```
#### Note for Linux users
##### You can execute scripts for download and build the necessary libraries after installing OpenSSL library (www.openssl.org/source/).
```
cd Progetto_PDS
cd Scripts_(linux_only)
# Boost installing
sudo ./BoostInstaller.sh
# OpenSSL installing 
# move to directory which contains the .tar.gz OpenSSL file and copy it OpenSSLInstaller.sh
sudo ./OpenSSLInstaller.sh
```
## Usage 
- Client Side
```
cd Progetto_PDS
cd client
./client <Folder_To_Track> <Server_IPaddress> <Port>
```
where Folder_To_Track is a path.
- Server Side
  - Configure users
  
      You can insert new users modifying /server/credenziali.txt (add or remove new line <username> <password>)
  
  - Backup directory
  
    /server/backup contains the backup folders for each user
    
  - Launch
  ```
  cd Progetto_PDS
  cd server
  ./server <Port>
  ```
## Description

### Client
#### ARCHITECTURE
* It receives three arguments: the path to back up, the IP and port of the backup server;
* Client has two threads, one acts as producer and the other as consumer;
* The communication between producer and consumer is managed by a thread-safe queue provided by the Boost library;
* In addition, there is a thread-safe map that is used for another purpose.

#### PRODUCER
* The producer consists in a file system watcher that notices all the changes in the path provided by the user;
* If a notable change is noted, the producer puts it into the queue;
* If the queue is full, it removes the oldest entry from the queue, puts the new one and set the map as to sync;
* The producer can be stopped using an appropriate method. 

#### CONSUMER
* First of all, it syncs the map if it is set as to be synced;
* Then if there is an entry in the queue, the consumer processes it sending to the server the message corresponding to the change in the file system;
* If there are any exceptions, they are caught: some has dedicated handlers, others the general one.


### Communication API
#### Client Side
Client builds a specific header to notify the server about the modifications detected by FileSystemWatcher.

The possible headers are : 
- UPDATE <path> <file_size> 
- UPDATE_NAME <path> <new_name>
- REMOVE <path>
- CREATE_FILE <path> <fileSize>
- CREATE_DIR <path>
- SYNC_DIR <path>
- SYNC_FILE <path> <hash_value>
- AUTH <username> 
- AUTH_CHALLENGE <iv_size> <cipher_challenge_size>

In addition the client sends to server body messages concerning the following headers:
- UPDATE and CREATE_FILE : the body messages contain file chunks  (the entire file is divided into one or more chuncks).
- AUTH_CHALLENGE : the body message contains the iv and the cipher challenge.

The client waits a server response which can be different depending to the situation.

The possible responses are:
- OK
- INTERNAL_ERROR
- OLD_VERSION
- NOT_PRESENT
- NON_AUTHENTICATED

#### Server Side
Server waits a message (header) from the client. Once received it, the server calls a specific method to execute the folder updating and then it sends to client a response.
The possible responses are:
- OK
- INTERNAL_ERROR
- OLD_VERSION
- NOT_PRESENT
- NON_AUTHENTICATED
