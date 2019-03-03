#include <string>
#include "spdlog/spdlog.h"
#include "site_config.hpp"
#include "update.hpp"
#include "retrieve.hpp"
#include "register.hpp"
#include "ping.hpp"
#include "remove.hpp"
#include "record_mgr.hpp"
#include "pistache/router.h"
#include "pistache/endpoint.h"

class EndpointManager {
    public:
        EndpointManager(std::shared_ptr<spdlog::logger>, SiteConfig*, RecordManager*);
        ~EndpointManager();
        bool start();
        void shutdown();
        bool initialize();
    private:
        bool configure();
        bool initHandlers();
        SiteConfig* config;
        PingEndpoint * ping_pt;
        UpdateEndpoint * update_pt;
        RegisterEndpoint * register_pt;
        RetrieveEndpoint * retrieve_pt;
        RemoveEndpoint * remove_pt;
        RecordManager * record_mgr;
        std::shared_ptr<spdlog::logger> logger;
        std::shared_ptr<Pistache::Http::Endpoint> httpEndpoint;
        Pistache::Rest::Router router;
};