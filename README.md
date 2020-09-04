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
  
    /server/backup contains the backup foldsers for each user
    
  - Launch
  ```
  cd Progetto_PDS
  cd server
  ./server
  ```
  
  
 