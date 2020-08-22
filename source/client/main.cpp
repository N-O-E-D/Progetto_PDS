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
PathStatusMap pathSyncStatus("../test");

void fileSystemWatcher() {
    // Create a FileWatcher instance that will check the current folder for changes every 5 seconds
    FileSystemWatcher fw{"../test", std::chrono::milliseconds(5000)};
    std::cout<<"fw created\n";
    // Start monitoring a folder for changes and (in case of changes)
    // run a user provided lambda function
    fw.start([] (const std::string& path_to_watch, Status status) -> void {
        // Process only regular files or directories
        if( !(std::filesystem::is_regular_file(std::filesystem::path(path_to_watch))
              || std::filesystem::is_directory(std::filesystem::path(path_to_watch)))
            && status != Status::Erased) {
            std::cout << "Not processed: " << path_to_watch << std::endl;
            return;
        }
        // Inset the path into queue, if it is full enter the if
        if(!path_to_process.push(make_pair(path_to_watch, status))){
            // Remove the oldest element and push the new one
            path_to_process.pop();
            path_to_process.push(make_pair(path_to_watch, status));
        }
        // Set path as NotSynced
        pathSyncStatus.setNotSynced(path_to_watch);
    });
}

void syncHandler(const std::string& path) {
   pathSyncStatus.setSynced(path);
}

void removeHandler(const std::string& path){
    pathSyncStatus.remove(path);
}

int main() {
    std::cout<<"Starting program\n";

    std::string address = "127.0.0.1";
    std::string port = "5000";

    // 1. Start File System Watcher
    std::thread t1{fileSystemWatcher};

    // 2. Open socket (MODIFICA AGGIUNTA DA LORENZO)
    boost::asio::io_service ioService;
    boost::asio::ip::tcp::resolver resolver(ioService);
    auto endpointIterator = resolver.resolve({ address, port });
    ClientSocket socket(ioService, endpointIterator);
    ioService.run();

    // 3. Consumer process
    std::pair<std::string, Status> path;
    while(true){
        // 3.1 Synchronization
        if(!pathSyncStatus.isSynced())
            pathSyncStatus.iterate_map([&socket] (const std::pair<std::string,SyncStatus>& path) -> void {
                // 3.1 If already synced return
                if( path.second == SyncStatus::Synced ) return;
                // 3.2 If not synced, do it
                if( std::filesystem::is_directory(std::filesystem::path(path.first)) )
                    socket.syncDir(path.first, syncHandler);
                else
                    socket.syncFile(path.first, syncHandler);
            });
        if(path_to_process.pop(path)){
            // 3.2 Send the corresponding message
            try{
                switch(path.second) {
                    case Status::FileCreated:
                        std::cout << "File created: " << path.first << '\n';
                        // Update server
                        socket.createFile(path.first, syncHandler);
                        break;
                    case Status::FileModified:
                        std::cout << "File modified: " << path.first << '\n';
                        // Update server
                        socket.update(path.first, syncHandler);
                        break;
                    case Status::Erased:
                        std::cout << "File or Directory erased: " << path.first << '\n';
                        // Update server
                        socket.remove(path.first, removeHandler);
                        break;
                    case Status::DirCreated:
                        std::cout << "Directory created: " << path.first << '\n';
                        // Update server
                        socket.createDir(path.first, syncHandler);
                        break;
                    default:
                        std::cout << "Error! Unknown file status.\n";
                }
            } catch (std::exception& e){
                std::cout << e.what() << std::endl;
            }
        }
    }
    t1.join();
    return 0;
}

