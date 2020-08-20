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

        // Insert path into the queue, if it is full returns false
        while(!path_to_process.push(make_pair(path_to_watch, status)));
        // Set path as NotSynced
        pathSyncStatus.setNotSynced(path_to_watch);
    });
}

int main() {
    std::cout<<"Starting program\n";

    std::string address = "127.0.0.1";
    std::string port = "5000";

    // 1. Start File System Watcher
    std::thread t1{fileSystemWatcher};

    // 2. Connect to the server
    //boost::asio::io_service ioService;
    //boost::asio::ip::tcp::resolver resolver(ioService);
    //auto endpointIterator = resolver.resolve({ address, port });
    //ClientSocket socket(ioService, endpointIterator);
    // 3. Authentication

    // 4. Synchronization
//    pathSyncStatus.iterate_map([&ioService,&socket] (const std::pair<std::string,SyncStatus>& path) -> void {
//        std::cout << "Checking: " << path.first << std::endl;
//        bool synced;
//        if( std::filesystem::is_directory(std::filesystem::path(path.first)) )
//            socket.syncDir(path.first); // messaggio di ritorno da parte del server: true/false
////        else
////            socket.syncFile(path.first);
//        ioService.run();
//        if(synced) pathSyncStatus.setSynced(path.first);
//    });
//
//    pathSyncStatus.iterate_map([&ioService,&socket] (const std::pair<std::string,SyncStatus>& path) -> void {
//        // tutto ciò non sincronizzato va inviato... Update/Remove/Add/AddDir
//        if(path.second == SyncStatus::Synced) return;
//    });

    // 5. Consumer process
    std::pair<std::string, Status> path;
    while(true){

        if(path_to_process.pop(path)){
            boost::asio::io_service ioService;
            boost::asio::ip::tcp::resolver resolver(ioService);
            auto endpointIterator = resolver.resolve({ address, port });
            ClientSocket socket(ioService, endpointIterator);
            std::cout<<"Path popped\n";
            bool done = false;
            //while(!done){
                std::cout<<"Into the done loop\n";
                try{
                    std::cout<<"Into try\n";
                    switch(path.second) {
                        case Status::FileCreated:
                            std::cout << "File created: " << path.first << '\n';
                            // Update server
                            socket.createFile(path.first);
                            ioService.run();
                            // set sync status as synced
                            pathSyncStatus.setSynced(path.first);
                            // break the loop
                            done = true;
                            break;
                        case Status::FileModified:
                            std::cout << "File modified: " << path.first << '\n';
                            // Update server
                            socket.update(path.first);
                            ioService.run();
                            // set sync status as synced
                            pathSyncStatus.setSynced(path.first);
                            // break the loop
                            done = true;
                            break;
                        case Status::Erased:
                            std::cout << "File or Directory erased: " << path.first << '\n';
                            // Update server
                            /*AGGIUNTA DI LORENZO : MANCAVA DISCRIMINAZIONE TRA FILE E DIRECTORY */
                            socket.remove(path.first);
                            ioService.run();
                            // set sync status as synced
                            pathSyncStatus.remove(path.first);
                            // break the loop
                            done = true;
                            break;
                        case Status::DirCreated:
                            std::cout << "Directory created: " << path.first << '\n';
                            // Update server
                            socket.createDir(path.first);
                            ioService.run();
                            // set sync status as synced
                            pathSyncStatus.setSynced(path.first);
                            // break the loop
                            done = true;
                            break;
                        default:
                            std::cout << "Error! Unknown file status.\n";
                            done = true;
                    }
                } catch (std::exception& e){
                    std::cout << e.what() << std::endl;
                //}
            }
        }
    }
    t1.join();
    return 0;
}
