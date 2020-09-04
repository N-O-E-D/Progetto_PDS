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
