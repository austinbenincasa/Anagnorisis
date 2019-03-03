#include "record_mgr.hpp"
#include <chrono>

RecordManager::RecordManager(std::shared_ptr<spdlog::logger> logger, SiteConfig* site_config) 
: updating(false), logger(logger), config(site_config) {

}

RecordManager::~RecordManager() {
    
}

void RecordManager::start() {
    logger->info("RecordManager: Starting manager");
    auto watcher = std::bind(&RecordManager::watcher, this);
    watcherThread = std::thread(watcher);
    auto updater = std::bind(&RecordManager::updater, this);
    updaterThread = std::thread(updater);
}

void RecordManager::shutdown() {
    logger->info("RecordManager: Shutting down");
    if(watcherThread.joinable()) {
        watcherThread.join();
    }
    if(updaterThread.joinable()) {
        updaterThread.join();
    }
}

bool RecordManager::recordExist(std::string name) {
    if(updating == true) {
        if (temp_record_map.count(name)) {
            return true;
        }
        else {
            return false;
        }
    }
    else {
        if (temp_record_map.count(name)) {
            return true;
        }
        else {
            return false;
        }
    }
}

std::string RecordManager::getResponseMsg(RecordResponse response) {
    switch (response)
    {
        case OK:
            return "Operation completed successfully";
        case IDERROR:
            return "Id is not valid";
        case IDMISMATCH:
            return "Id is not associated with a record";
        case NAMEERROR:
            return "Name is already associated to a record";
        case NORECORD:
            return "No record found";
        default:
            return "Unknown Server Error";
    }
}

int RecordManager::getErrorCode(RecordResponse response) {
    switch (response)
    {
        case OK:
            return 0;
        case IDERROR:
            return 1;
        case IDMISMATCH:
            return 2;
        case NAMEERROR:
            return 3;
        case NORECORD:
            return 4;
        default:
            return -1;
    }
}

RecordResponse RecordManager::pingRecord(std::string id, std::string name) {
    if(updating == true) {
        if (temp_record_map.count(name) > 0) {
            pings.push(std::make_tuple (0, id)); 
            return OK;
        }
        else {
            return NORECORD;
        }
    }
    else {
        if (record_map.count(name) > 0) {
            pings.push(std::make_tuple (0, id)); 
            return OK;
        }
        else {
            return NORECORD;
        }
    }  
}

RecordResponse RecordManager::retrieveRecord(std::string name, std::string& data) {
    if(updating == true) {
        if (temp_record_map.count(name) > 0) {
            data = temp_record_map[name];
            return OK;
        }
        else {
            data = "";
            return NORECORD;
        }
    }
    else {
        if (record_map.count(name) > 0) {
            data = record_map[name];
            return OK;
        }
        else {
            data = "";
            return NORECORD;
        }
    }
}

RecordResponse RecordManager::updateRecord(std::string id, std::string name, std::string record) {
    if(updating == true) {
        if (temp_id_map.count(id) > 0) {
            if(name == temp_id_map[id]) {
                update_queue.push(std::make_tuple (1, id, name, record));
                return OK;
            }
            else {
                return IDMISMATCH;
            }
        }
        else {
            return IDERROR;
        }
    }
    else {
        if (id_map.count(id) > 0) {
            if(name == id_map[id]) {
                update_queue.push(std::make_tuple (1, id, name, record));
                return OK;
            }
            else {
                return IDMISMATCH;
            }
        }
        else {
            return IDERROR;
        }
    }
}

RecordResponse RecordManager::createRecord(std::string id, std::string name, std::string record) {
    if(updating == true) {
        if (temp_id_map.count(id) > 0) {
            return IDMISMATCH;
        }
        if (temp_record_map.count(name)) {
            return NAMEERROR;
        }
        else {
            update_queue.push(std::make_tuple (0, id, name, record));
            return OK;
        }
    }
    else {
        if (id_map.count(id) > 0) {
            return IDMISMATCH;
        }
        if (record_map.count(name)) {
            return NAMEERROR;
        }
        else {
            update_queue.push(std::make_tuple (0, id, name, record));
            return OK;
        }
    }
}

RecordResponse RecordManager::removeRecord(std::string id, std::string name) {
    if (id_map.count(id) > 0) {
        if(updating == true) {
            if(temp_id_map[id] == name) {
                update_queue.push(std::make_tuple (2, id, name, ""));
                return OK;
            }
            else {
                return IDMISMATCH;
            }
        }
        else {
            if(id_map[id] == name) {
                update_queue.push(std::make_tuple (2, id, name, ""));
                return OK;
            }
            else {
                return IDMISMATCH;
            }
        }
    }
    else {
        return IDERROR;
    }
}

void RecordManager::clearRecords() {
    glock.lock();
    updating = true;
    record_map.clear();
    updating = false;
    temp_record_map.clear();
    glock.unlock();
}

unsigned char RecordManager::random_char() {
    std::random_device rd;
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> dis(0, 255);
    return static_cast<unsigned char>(dis(gen));
}

std::string RecordManager::generateUUID(const unsigned int len) {
    std::stringstream ss;
    for(auto i = 0; i < len; i++) {
        auto rc = random_char();
        std::stringstream hexstream;
        hexstream << std::hex << int(rc);
        auto hex = hexstream.str(); 
        ss << (hex.length() < 2 ? '0' + hex : hex);
    }        
    return ss.str();
}

void RecordManager::updater() {
    logger->info("RecordManager: Updater thread started");
    std::tuple<int, std::string, std::string, std::string> record;
    int timeout = config->get_core("updater_timeout", 100);
    int records = 0;
    int updated = 0;
    int removed = 0;
    int added = 0;
    while(running) {
        records = update_queue.size(); 
        if(records > 0) {
            glock.lock();
            updating = true;
            while(update_queue.size() > 0) {
                record = update_queue.front();
                update_queue.pop();
                if(std::get<0>(record) == 0) { //add
                    pings.push(std::make_tuple (0, std::get<1>(record)));
                    id_map[std::get<1>(record)] = std::get<2>(record);
                    record_map[std::get<2>(record)] = std::get<3>(record);
                    added++;
                }
                else if(std::get<0>(record) == 1) { //updated
                    pings.push(std::make_tuple (0, std::get<1>(record)));                    
                    record_map[std::get<2>(record)] = std::get<3>(record);
                    updated++;
                }
                else if(std::get<0>(record) == 2) {
                    pings.push(std::make_tuple (1, std::get<1>(record)));
                    id_map.erase(std::get<1>(record));
                    record_map.erase(std::get<2>(record));
                    removed++;
                }
            }
            temp_record_map = record_map;
            temp_id_map = id_map;
            updating = false;
            glock.unlock();
            if(added > 0) {
                logger->info("RecordManager: Added {} records", added);
                logger->info("RecordManager: Total size {} records", record_map.size());
                added = 0; 
            }
            if(removed > 0) {
                logger->info("RecordManager: Removed {} records", removed);
                logger->info("RecordManager: Total size {} records", record_map.size());
                removed = 0; 
            }
            if(updated > 0) {
                logger->info("RecordManager: Updated {} records", updated); 
                updated = 0;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
    }
    logger->info("RecordManager: Updater thread stopped");
}



void RecordManager::watcher() {
    logger->info("RecordManager: Watcher thread started");
    time_t now;
    std::string name;
    int timeout = config->get_core("watcher_timeout", 5000);
    std::tuple<int, std::string> ping;
    while(running) {
        if(pings.size() > 0) {
            while(pings.size() > 0) {
                ping = pings.front();
                pings.pop();
                if(std::get<0>(ping) == 0) {
                    record_time[std::get<1>(ping)] = time(NULL);
                }
                else if(std::get<0>(ping) == 1) {
                    record_time.erase(std::get<1>(ping));
                }
                else {
                    logger->warn("RecordManager: Received invalid ping operation");
                }
            }
        }
        now = time(NULL);
        for (auto const& entry : record_time) {
            if((now - entry.second) > 10) {
                if(updating == true) {
                    name = temp_id_map[entry.first];
                    //logger->info("RecordManager: Record '{}' expired: removing record", name);
                    removeRecord(entry.first, name);
                }
                else {
                    name = id_map[entry.first];
                    //logger->info("RecordManager: Record '{}' expired: removing record", name);
                    removeRecord(entry.first, name);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
    }
    logger->info("RecordManager: Watcher thread stopped");
}

