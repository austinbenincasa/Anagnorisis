#pragma once
#include <map>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>
#include <sstream>
#include <random>
#include <climits>
#include <algorithm>
#include <functional>
#include <string>
#include "site_config.hpp"
#include "spdlog/spdlog.h"

enum RecordResponse {
    OK = 0,
    IDERROR = 1,
    IDMISMATCH = 2,
    NAMEERROR = 3,
    NORECORD = 4,
};

class RecordManager {
    public:
        RecordManager(std::shared_ptr<spdlog::logger>, SiteConfig*);
        ~RecordManager();
        bool recordExist(std::string);
        std::string generateUUID(const unsigned int);
        RecordResponse pingRecord(std::string, std::string);
        RecordResponse retrieveRecord(std::string, std::string&);
        RecordResponse createRecord(std::string, std::string, std::string);
        RecordResponse updateRecord(std::string, std::string, std::string);
        RecordResponse removeRecord(std::string, std::string);
        std::string getResponseMsg(RecordResponse);
        int getErrorCode(RecordResponse);
        void clearRecords();
        void start();
        void shutdown();
    private:
        void watcher();
        void updater();
        unsigned char random_char();
        bool running = true;
        SiteConfig* config;
        std::mutex glock;
        std::atomic<bool> updating;
        std::thread watcherThread;
        std::thread updaterThread;
        std::shared_ptr<spdlog::logger> logger;
        std::map<std::string, std::string> temp_record_map;
        std::map<std::string, std::string> record_map;
        std::map<std::string, std::string> id_map;
        std::map<std::string, std::string> temp_id_map;
        std::map<std::string, time_t> record_time;
        std::queue<std::tuple<int, std::string>> pings;
        std::queue<std::tuple<int, std::string, std::string, std::string>> update_queue;
};
