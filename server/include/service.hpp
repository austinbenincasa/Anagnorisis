#include <string>
#include <ctime>
#include "site_config.hpp"
#include "record_mgr.hpp"
#include "endpoint_mgr.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

enum ServiceState { 
    starting = 0,
    running = 1,
    stopping = 2,
    stopped = 3,
    restarting = 4,
    crashed = 5
};

class Service {
    public:
        Service(std::string);
        void stop();
        void start();
        void restart();
        void cleanup();
        ServiceState getState();
    private:
        void run();
        void init_logging();
        bool init_classes();
        std::string getUptime(time_t&);
        time_t start_time;
        ServiceState state;
        std::shared_ptr<spdlog::logger> logger;
        RecordManager* record_mgr;
        SiteConfig* config;
        EndpointManager* endpoint_mgr;
        std::string config_path;
};