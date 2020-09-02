//
// Created by Giandonato Farina on 11/08/2020.
//

#pragma once

#include <map>
#include <string>
#include <mutex>
#include <filesystem>

// Class that defines synchornization status
enum class SyncStatus { Synced, NotSynced };

class PathStatusMap {
private:
    // Private variables
    std::map<std::string, SyncStatus> map;
    bool toSync = true;
    std::mutex m;
public:
    // Public methods

    /**
     * Populates the map recursively starting from the root path
     * @param root_path
     */
    void setRoot(const std::string& root_path){
        this->insert(root_path, SyncStatus::NotSynced);
        for (auto &file : std::filesystem::recursive_directory_iterator(root_path))
            this->insert(file.path().string(), SyncStatus::NotSynced);
    }

    /**
     * Insert a new entry into the map
     * @param key
     * @param value
     */
    void insert(const std::string& key, SyncStatus value) {
        std::unique_lock ul (m);
        map.insert(std::make_pair(key, value));
    }

    /**
     * Remove an entry from the map
     * @param key
     */
    void remove(const std::string& key) {
        std::unique_lock ul (m);
        map.erase(key);
    }

    /**
     * Execute the action on all entries of the map
     * @tparam P
     * @param action
     */
    template <class P>
    void iterate_map(P action) {
        std::unique_lock ul (m);
        for(auto& it : map) {
            ul.unlock();
            action(it);
            ul.lock();
        }
    }

    /**
     * Set the value corresponding to the key to Synced
     * @param key
     */
    void setSynced(const std::string& key) {
        std::unique_lock ul (m);
        map[key] = SyncStatus::Synced;
    }

    /**
     * Set the value corresponding to the key to Not Synced
     * @param key
     */
    void setNotSynced(const std::string& key) {
        std::unique_lock ul (m);
        map[key] = SyncStatus::NotSynced;
    }

    /**
     * Return true if the map is to be synced
     * @return
     */
    bool isToSync(){
        std::unique_lock ul (m);
        return toSync;
    }

    /**
     * Set the map as to sync
     */
    void setToSync(){
        std::unique_lock ul (m);
        toSync = true;
    }

    /**
     * Set the map as not to sync
     */
    void setNotToSync(){
        std::unique_lock ul (m);
        toSync = false;
    }

    /**
     * Write the content of the map to the console
     */
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