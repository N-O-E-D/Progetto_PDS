//
// Created by Giandonato Farina on 11/08/2020.
//

#pragma once

#include <map>
#include <string>
#include <mutex>
#include <filesystem>

enum class SyncStatus { Synced, NotSynced };

class PathStatusMap {
private:
    std::map<std::string, SyncStatus> map;
    bool toSync = true;
    std::mutex m;
public:
    void setRoot(const std::string& root_path){
        this->insert(root_path, SyncStatus::NotSynced);
        for (auto &file : std::filesystem::recursive_directory_iterator(root_path))
            this->insert(file.path().string(), SyncStatus::NotSynced);
    }

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
        for(auto& it : map) {
            ul.unlock();
            action(it);
            ul.lock();
        }
    }

    void setSynced(const std::string& key) {
        std::unique_lock ul (m);
        map[key] = SyncStatus::Synced;
    }

    void setNotSynced(const std::string& key) {
        std::unique_lock ul (m);
        map[key] = SyncStatus::NotSynced;
    }

//    bool isSynced() {
//        std::unique_lock ul (m);
//        for(auto& it : map) {
//            if (it.second == SyncStatus::NotSynced)
//                return false;
//        }
//        return true;
//    }

    bool isToSync(){
        std::unique_lock ul (m);
        return toSync;
    }

    void setToSync(){
        std::unique_lock ul (m);
        toSync = true;
    }

    void setNotToSync(){
        std::unique_lock ul (m);
        toSync = false;
    }

    void print() {
        std::unique_lock ul(m);
        for(auto& it : map) {
            std::cout << it.first << " ";
            if(it.second == SyncStatus::Synced)
                std::cout << "Synced\n";
            else
                std::cout << "Not Synced\n";
        }
    }
};