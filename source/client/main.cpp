//
// Created by Giandonato Farina on 24/07/2020.
//

#include <boost/lockfree/spsc_queue.hpp>
#include <thread>
#include <iostream>

#include "FileSystemWatcher.h"
#include "PathStatusMap.h"
#include "./API_client/ClientSocket.h"

#define MAX_ATTEMPTS 3

boost::lockfree::spsc_queue< std::pair<std::string,Status> > path_to_process(1024);
PathStatusMap pathSyncStatus;

/**
 * Run handlers and reset ioService
 * @param ioService
 */
void runHandlers(boost::asio::io_service& ioService){
    ioService.run();
    ioService.reset();
}

/**
 * Start the file system watcher with the handler needed for this project
 * @param fw
 */
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

/**
 * Write the start screen on the console
 */
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

/**
 * Program receives as arguments root folder, ip address and port. Returns only in case of error
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char** argv) {
    // 1.1 Checking parameters
    if (argc < 4) {
        std::cerr << "Not enough parameters\n";
        return -1;
    }
    auto root = std::string(argv[1]);
    auto address = std::string(argv[2]);
    auto port = std::string(argv[3]);
    // 1.2 Initialization
    pathSyncStatus.setRoot(root);
    initScreen(); // modified by Lorenzo
    int attempts = 0;

    // 2. Start File System Watcher
    FileSystemWatcher fw{root, std::chrono::milliseconds(5000)};
    std::thread t1 {fswHandler, &fw};

    // 3. Open socket (MODIFIED BY LORENZO)
    boost::asio::io_service ioService;
    boost::asio::ip::tcp::resolver resolver(ioService);
    auto endpointIterator = resolver.resolve({address, port});
    ClientSocket socket(ioService, endpointIterator);

    // 4. Get credentials
    std::string username;
    std::string password;
    // 4.1 Ask user to insert credentials
    std::cout << "Please insert username: ";
    std::cin >> username;
    std::cout << "Please insert password: ";
    std::cin >> password;
    // 4.2 Set credentials
    socket.setUsername(username);
    socket.setPassword(password);

    // 5. Main process

    std::pair<std::string, Status> path;
    while(true){
        try{
            // 5.1 Synchronization
            if(pathSyncStatus.isToSync()) {
                pathSyncStatus.iterate_map([&socket, &ioService](const std::pair<std::string, SyncStatus> &path) -> void {
                    // 5.1.1 If already synced return
                    if (path.second == SyncStatus::Synced) return;
                    // 5.1.2 If not synced, do it
                    log(CLIENT,WAIT,"Trying to sync : "+ path.first);
                    if (std::filesystem::is_directory(std::filesystem::path(path.first)))
                        socket.syncDir(path.first);
                    else
                        socket.syncFile(path.first);

                    runHandlers(ioService);
                    // 5.1.3 Set synced in the map
                    pathSyncStatus.setSynced(path.first);
                });
                // 5.2 When all is synced unflag
                pathSyncStatus.setNotToSync();
            }
            // 5.2 If queue is not empty, process one entry
            if(path_to_process.pop(path)){
                // 5.2.1 Send the corresponding message and update map
                switch(path.second) {
                    case Status::FileCreated:
                        log(CLIENT,WAIT, "File created: " + path.first);
                        socket.createFile(path.first);
                        runHandlers(ioService);
                        pathSyncStatus.setSynced(path.first);
                        break;
                    case Status::FileModified:
                        log(CLIENT,WAIT, "File modified: " + path.first);
                        socket.update(path.first);
                        runHandlers(ioService);
                        pathSyncStatus.setSynced(path.first);
                        break;
                    case Status::Erased:
                        log(CLIENT,WAIT, "File or Directory erased: " + path.first);
                        socket.remove(path.first);
                        runHandlers(ioService);
                        pathSyncStatus.remove(path.first);
                        break;
                    case Status::DirCreated:
                        log(CLIENT,WAIT, "Directory created: " + path.first);
                        socket.createDir(path.first);
                        runHandlers(ioService);
                        pathSyncStatus.setSynced(path.first);
                        break;
                }
            }
        }
        // 6. Exceptions handling
        // 5.1 Wrong username
        catch (WrongUsernameException& e){
            if(attempts++ < MAX_ATTEMPTS){
                // retry
                //std::cout << "Invalid username. " << MAX_ATTEMPTS-attempts+1 << " attempts remaining.\nPlease re-insert username: ";
                log(CLIENT,ERROR,"Invalid username. "+std::to_string(MAX_ATTEMPTS-attempts+1)+ " attempts remaining. Please re-insert username");
                std::cin >> username;
                socket.setUsername(username);
            } else {
                // stop
                fw.stop();
                log(CLIENT,ERROR,"Wrong username. No attempts remaining. Terminating program");
                t1.join();
                exit(201); // 201 => Wrong username
            }
        }
        // 5.2 Wrong password
        catch(WrongPasswordException& e){
            if(attempts++ < MAX_ATTEMPTS){
                // retry
                //std::cout << "Invalid password. " << MAX_ATTEMPTS-attempts+1 << " attempts remaining.\nPlease re-insert password: ";
                log(CLIENT,ERROR,"Invalid password. "+std::to_string(MAX_ATTEMPTS-attempts+1)+ " attempts remaining. Please re-insert password");
                std::cin >> password;
                socket.setPassword(password);
            } else {
                // stop
                fw.stop();
                log(CLIENT,ERROR,"Wrong password. No attempts remaining. Terminating program");
                t1.join();
                exit(202); // 202 => Wrong password
            }
        }
        // 5.3 Generic exception
        catch (std::exception& e){
            // stop
            fw.stop();
            log(CLIENT,ERROR, e.what());
            t1.join();
            exit(200); // 200 => generic error
        }
    }
}

