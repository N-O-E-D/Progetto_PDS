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

    //1.1 OpenSocket (MODIFICA AGGIUNTA DA LORENZO)
    boost::asio::io_service ioService;
    boost::asio::ip::tcp::resolver resolver(ioService);
    auto endpointIterator = resolver.resolve({ address, port });
    ClientSocket socket(ioService, endpointIterator);
    ioService.run();
    // 2. Synchronization -> problem with async communication
    pathSyncStatus.iterate_map([address, port] (const std::pair<std::string,SyncStatus>& path) -> void {
        // 2.1 If already synced return
        if( path.second == SyncStatus::Synced ) return;

        // 2.2 If not synced, do it
        if( std::filesystem::is_directory(std::filesystem::path(path.first)) )
            socket.syncDir(path.first);
        else
            socket.syncFile(path.first);

        pathSyncStatus.setSynced(path.first);
    });

    // 3. Consumer process
    std::pair<std::string, Status> path;
    while(true){
        if(path_to_process.pop(path)){
            // 3.1 Open socket
            //Open Socket già fatta (MODIFICA DI LORENZO)

            // 3.2 Send the corresponding message
            try{
                switch(path.second) {
                    case Status::FileCreated:
                        std::cout << "File created: " << path.first << '\n';
                        // Update server
                        socket.createFile(path.first);
                        //ioService.run();
                        // set sync status as synced
                        pathSyncStatus.setSynced(path.first);
                        break;
                    case Status::FileModified:
                        std::cout << "File modified: " << path.first << '\n';
                        // Update server
                        socket.update(path.first);
                        //ioService.run();
                        // set sync status as synced
                        pathSyncStatus.setSynced(path.first);
                        break;
                    case Status::Erased:
                        std::cout << "File or Directory erased: " << path.first << '\n';
                        // Update server
                        socket.remove(path.first);
                        //ioService.run();
                        // set sync status as synced
                        pathSyncStatus.remove(path.first);
                        break;
                    case Status::DirCreated:
                        std::cout << "Directory created: " << path.first << '\n';
                        // Update server
                        socket.createDir(path.first);
                        //ioService.run();
                        // set sync status as synced
                        pathSyncStatus.setSynced(path.first);
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
