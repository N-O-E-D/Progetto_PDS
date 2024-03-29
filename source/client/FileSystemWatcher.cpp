//
// Created by Giandonato Farina on 23/07/2020.
//

#include "FileSystemWatcher.h"

#include <iostream>

/**
 * FileSystemWatcher constructor
 * @param path_to_watch
 * @param delay
 */
FileSystemWatcher::FileSystemWatcher(const std::string& path_to_watch, std::chrono::duration<int, std::milli> delay) : path_to_watch{path_to_watch}, delay{delay} {
    for(auto &file : std::filesystem::recursive_directory_iterator(path_to_watch))
        paths_[file.path().string()] = std::filesystem::last_write_time(file);
}

/**
 * Start the file system watcher and call the action
 * @param action
 */
void FileSystemWatcher::start(const std::function<void(std::string, Status)> &action) {
    std::unique_lock ul (m);
    while(running_) {
        ul.unlock();
        // Wait for "delay" milliseconds
        std::this_thread::sleep_for(delay);
        auto it = paths_.begin();
        while (it != paths_.end()) {
            if (!std::filesystem::exists(it->first)) {  // if path does not exist => file/dir2 erased
                action(it->first, Status::Erased);
                it = paths_.erase(it);
            }
            else {
                it++;
            }
        }
        // Check if a file was created or modified
        for(auto &file : std::filesystem::recursive_directory_iterator(path_to_watch)) {
            auto current_file_last_write_time = std::filesystem::last_write_time(file);
            // File creation
            if(!contains(file.path().string())) {
                paths_[file.path().string()] = current_file_last_write_time;
                if(file.is_directory())
                    action(file.path().string(), Status::DirCreated);
                else
                    action(file.path().string(), Status::FileCreated);
            }
            // File modification
            else {
                if(paths_[file.path().string()] != current_file_last_write_time) {
                    paths_[file.path().string()] = current_file_last_write_time;
                    if(!file.is_directory())    // Ignoring directory modifications
                        action(file.path().string(), Status::FileModified);
                }
            }
        }
        ul.lock();
    }
}

/**
 * Return true if the map contains the key
 * @param key
 * @return
 */
bool FileSystemWatcher::contains(const std::string &key) {
    auto el = paths_.find(key);
    return el != paths_.end();
}

/**
 * Stop the file system watcher
 */
void FileSystemWatcher::stop() {
    std::unique_lock ul (m);
    running_ = false;
}



