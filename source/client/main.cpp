//
// Created by bruno on 14/07/2020.
//

#include <iostream>

#include "FileSystemWatcher.h"


int main() {
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

        switch(status) {
            case Status::FileCreated:
                std::cout << "File created: " << path_to_watch << '\n';
                break;
            case Status::FileModified:
                std::cout << "File modified: " << path_to_watch << '\n';
                break;
            case Status::Erased:
                std::cout << "File or Directory erased: " << path_to_watch << '\n';
                break;
            case Status::DirCreated:
                std::cout << "Directory created: " << path_to_watch << '\n';
                break;
            default:
                std::cout << "Error! Unknown file status.\n";
        }
    });
}
