//
// Created by Giandonato Farina on 24/07/2020.
//

#include <boost/lockfree/spsc_queue.hpp>
#include <thread>
#include <iostream>

#include "FileSystemWatcher.h"

boost::lockfree::spsc_queue< std::pair<std::string,Status> > path_to_process(1024) ;

void fileSystemWatcher() {
    // Create a FileWatcher instance that will check the current folder for changes every 5 seconds
    FileSystemWatcher fw{"../test", std::chrono::milliseconds(5000)};

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
    });
}

int main() {
    std::thread t1{fileSystemWatcher};

    std::pair<std::string, Status> path;
    while(true){
        if(path_to_process.pop(path)){
            switch(path.second) {
                case Status::FileCreated:
                    std::cout << "File created: " << path.first << '\n';
                    break;
                case Status::FileModified:
                    std::cout << "File modified: " << path.first << '\n';
                    break;
                case Status::Erased:
                    std::cout << "File or Directory erased: " << path.first << '\n';
                    break;
                case Status::DirCreated:
                    std::cout << "Directory created: " << path.first << '\n';
                    break;
                default:
                    std::cout << "Error! Unknown file status.\n";
            }
        }
    }

    t1.join();
    return 0;
}