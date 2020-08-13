//
// Created by Giandonato Farina on 23/07/2020.
//

#pragma once

#include <string>
#include <chrono>
#include <filesystem>
#include <unordered_map>
#include <thread>
#include <functional>

// Define all possible changes
enum class Status { FileCreated, FileModified, Erased, DirCreated};

class FileSystemWatcher {
public:
    std::string path_to_watch;
    std::chrono::duration<int, std::milli> delay;

    FileSystemWatcher(const std::string& path_to_watch, std::chrono::duration<int, std::milli> delay);
    void start(const std::function<void (std::string, Status)> &action);

private:
    std::unordered_map<std::string, std::filesystem::file_time_type> paths_;
    bool running_ = true;

    bool contains(const std::string &key);
};

