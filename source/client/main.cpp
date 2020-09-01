//
// Created by Giandonato Farina on 24/07/2020.
//

#include <boost/lockfree/spsc_queue.hpp>
#include <thread>
#include <iostream>

#include "FileSystemWatcher.h"
#include "PathStatusMap.h"
#include "../API_client/ClientSocket.h"

boost::lockfree::spsc_queue< std::pair<std::string,Status> > path_to_process(1024);
PathStatusMap pathSyncStatus;

void runHandlers(boost::asio::io_service& ioService){
    ioService.run();
    ioService.reset();
}

//void fileSystemWatcher(const std::string& root) {
//    // Create a FileWatcher instance that will check the current folder for changes every 5 seconds
//    FileSystemWatcher fw{root, std::chrono::milliseconds(5000)};
//    // Start monitoring a folder for changes and (in case of changes)
//    // run a user provided lambda function
//    fw.start([] (const std::string& path_to_watch, Status status) -> void {
//        // Process only regular files or directories
//        if( !(std::filesystem::is_regular_file(std::filesystem::path(path_to_watch))
//              || std::filesystem::is_directory(std::filesystem::path(path_to_watch)))
//            && status != Status::Erased)
//            return;
//
//        // Inset the path into queue, if it is full enter the if
//        if(!path_to_process.push(make_pair(path_to_watch, status))){
//            // Remove the oldest element and push the new one
//            path_to_process.pop();
//            path_to_process.push(make_pair(path_to_watch, status));
//            pathSyncStatus.setToSync();
//        }
//        // Set path as NotSynced
//        pathSyncStatus.setNotSynced(path_to_watch);
//    });
//}

void fswHandler (FileSystemWatcher* fw){
    fw->start([] (const std::string& path_to_watch, Status status) -> void {
        // Process only regular files or directories
        if( !(std::filesystem::is_regular_file(std::filesystem::path(path_to_watch))
              || std::filesystem::is_directory(std::filesystem::path(path_to_watch)))
            && status != Status::Erased)
            return;

        // Inset the path into queue, if it is full enter the if
        if(!path_to_process.push(make_pair(path_to_watch, status))){
            // Remove the oldest element and push the new one
            path_to_process.pop();
            path_to_process.push(make_pair(path_to_watch, status));
            pathSyncStatus.setToSync();
        }
        // Set path as NotSynced
        pathSyncStatus.setNotSynced(path_to_watch);
    });
}


void initScreen(){
    std::cout << R"(
 _____________________________________________________________________________
 |                                                                           |
 |   ______                     _        ______            _                 |
 |   | ___ \                   | |       | ___ \          | |                |
 |   | |_/ /___ _ __ ___   ___ | |_ ___  | |_/ / __ _  ___| | ___   _ _ __   |
 |   |    // _ \ '_ ` _ \ / _ \| __/ _ \ | ___ \/ _` |/ __| |/ / | | | '_ \  |
 |   | |\ \  __/ | | | | | (_) | ||  __/ | |_/ / (_| | (__|   <| |_| | |_) | |
 |   \_| \_\___|_| |_| |_|\___/ \__\___| \____/ \__,_|\___|_|\_\\__,_| .__/  |
 |                                                                   | |     |
 |                                                                   |_|     |
 |                                                                           |
 |                                                                           |
 |   Developed by Lorenzo Ceccarelli, Giandonato Farina, Bruno Accogli       |
 |___________________________________________________________________________|


    )" << "\n\n";
}

// program receive parameters as arguments
// program folder address port [time]
int main(int argc, char** argv) {
    // 0.1 Checking parameters
    if (argc < 4) {
        std::cerr << "Not enough parameters\n";
        return -1;
    }
    auto root = std::string(argv[1]);
    auto address = std::string(argv[2]);
    auto port = std::string(argv[3]);
    // 0.2 Initialization
    pathSyncStatus.setRoot(root);
    initScreen(); //modified by Lorenzo
    // 1. Start File System Watcher
    FileSystemWatcher fw{root, std::chrono::milliseconds(5000)};
    std::thread t1 {fswHandler, &fw};

//    std::thread t1{fileSystemWatcher, root};

    // 2. Open socket (MODIFIED BY LORENZO)
    boost::asio::io_service ioService;
    boost::asio::ip::tcp::resolver resolver(ioService);
    auto endpointIterator = resolver.resolve({address, port});
    ClientSocket socket(ioService, endpointIterator);

    // 3. Authentication
    std::string username;
    std::string password;
    // 3.1 Ask user to insert credentials
    std::cout << "Please insert username: ";
    std::cin >> username;
    std::cout << "Please insert password: ";
    std::cin >> password;
    socket.setUsername(username);
    socket.setPassword(password);
    // 3.2 Call authentication method
    /*if(socket.authenticate(username, password) != responseType::OK) {
        std::cerr << "Authentication error" << std::endl;
        return -2;
    }*/

    // 5. Consumer process
    std::pair<std::string, Status> path;
    while(true){
        // 5.1 Synchronization
        if(pathSyncStatus.isToSync()) {
            pathSyncStatus.iterate_map([&socket, &ioService, &username, &password](
                    const std::pair<std::string, SyncStatus> &path) -> void {
                // 5.1.1 If already synced return
                if (path.second == SyncStatus::Synced) return;

                /*PROVISIONALLY: do authentication */
                /*if (socket.authenticate(username, password) != responseType::OK) {
                    std::cerr << "Authentication error" << std::endl;
                    exit(255);
                }*/

                std::cout << "Trying to sync " << path.first << "\n";

                // 5.1.2 If not synced, do it
                if (std::filesystem::is_directory(std::filesystem::path(path.first)))
                    socket.syncDir(path.first);
                else {
                    log(TRACE,"Iterate map sync file");
                    socket.syncFile(path.first);
                }
                runHandlers(ioService);
                // 5.1.3 Set synced in the map
                pathSyncStatus.setSynced(path.first);
            });
            // 5.2 When all is synced unflag
            pathSyncStatus.setNotToSync();
        }
        // 5.2 If queue is not empty, process one entry
        if(path_to_process.pop(path)){
            /* PROVISIONALLY: do authentication */
            /*if(socket.authenticate(username, password) != responseType::OK) {
                std::cerr << "Authentication error" << std::endl;
                return -2;
            }*/
            try{
                // 5.2.1 Send the corresponding message and update map
//                // 3.2 Call authentication method
//                if(socket.authenticate(username, password) != responseType::OK) {
//                    std::cerr << "Authentication error" << std::endl;
//                    return -2;
//                }
                //throw std::logic_error("Exception test");
                switch(path.second) {
                    case Status::FileCreated:
                        log(TRACE, "File created: " + path.first);
                        socket.createFile(path.first);
                        runHandlers(ioService);
                        pathSyncStatus.setSynced(path.first);
                        break;
                    case Status::FileModified:
                        log(TRACE, "File modified: " + path.first);
                        socket.update(path.first);
                        runHandlers(ioService);
                        pathSyncStatus.setSynced(path.first);
                        break;
                    case Status::Erased:
                        log(TRACE, "File or Directory erased: " + path.first);
                        socket.remove(path.first);
                        runHandlers(ioService);
                        pathSyncStatus.remove(path.first);
                        break;
                    case Status::DirCreated:
                        log(TRACE, "Directory created: " + path.first);
                        socket.createDir(path.first);
                        runHandlers(ioService);
                        pathSyncStatus.setSynced(path.first);
                        break;
                }
            }
            catch (WrongUsernameException& e){
                fw.stop();
                t1.join();
                log(ERROR,e.what());
                exit(200);
            }
            catch(WrongPasswordException& e){
                fw.stop();
                t1.join();
                log(ERROR,e.what());
                exit(200);
            }
            catch (std::exception& e){
                fw.stop();
                t1.join();
                log(ERROR, e.what());
                exit(200);
            }
        }
    }
}

