//
// Created by Giandonato Farina on 11/08/2020.
//

#pragma once

#include <unordered_map>
#include <string>
#include <mutex>
#include <filesystem>

enum class SyncStatus { Synced, NotSynced };

class PathStatusMap {
private:
    std::unordered_map<std::string, SyncStatus> map;
    std::mutex m;
public:
    explicit PathStatusMap(const std::string& root_path) {
        std::cout<<"PathStatusMap constructor\n";
        for (auto &file : std::filesystem::recursive_directory_iterator(root_path))
            this->insert(file.path().string(), SyncStatus::NotSynced);
    };

    void insert(const std::string& key, SyncStatus value) {
        std::unique_lock ul (m);
        map.insert(std::make_pair(key, value));
    }

    void remove(const std::string& key) {
        std::unique_lock ul (m);
        map.erase(key);
    }

    template <class P>
    void iterate_map(P action) {
        std::unique_lock ul (m);
        std::for_each(map.begin(), map.end(), action);
    }

    void setSynced(const std::string& key) {
        std::unique_lock ul (m);
        map[key] = SyncStatus::Synced;
    }

    void setNotSynced(const std::string& key) {
        std::unique_lock ul (m);
        map[key] = SyncStatus::NotSynced;
    }
};