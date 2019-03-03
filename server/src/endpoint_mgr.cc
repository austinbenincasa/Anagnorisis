#include "endpoint_mgr.hpp"

using namespace Pistache;

EndpointManager::EndpointManager(std::shared_ptr<spdlog::logger> logger,  SiteConfig* site_config, RecordManager* record_mgr) 
: logger(logger), config(site_config), record_mgr(record_mgr) {

}

EndpointManager::~EndpointManager() {
    
}

bool EndpointManager::start() {
    logger->info("EndpointManager is starting RestfulHTTP server");
    try {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serveThreaded();
        return true;
    }
    catch(const std::exception& e) {
        logger->error("Failed to start EndpointManager RestfulHTTP server");
        return false;
    }
}

bool EndpointManager::initialize() {
    logger->info("EndpointManager is initializing");
    if(!initHandlers()) {
        return false;
    }
    if(!configure()) {
        return false;
    }
    Address addr(Ipv4::any(), config->get_core("port", 8080));
    httpEndpoint = std::make_shared<Http::Endpoint>(addr);
    try {
        auto opts = Http::Endpoint::options()
            .threads(config->get_core("threads", 2))
            .flags(Pistache::Tcp::Options::InstallSignalHandler);
        httpEndpoint->init(opts);
    }
    catch(const std::exception& e) {
        logger->error("Failed to initialize EndpointManager");
        return false;
    }
    return true;
}
void EndpointManager::shutdown() {
    logger->info("EndpointManager is shutting down");
    if(httpEndpoint != NULL) {
        httpEndpoint->shutdown();
    }
    if (ping_pt != NULL) {
        delete ping_pt;
    }
    if (update_pt != NULL) {
        delete update_pt;
    }
    if (register_pt != NULL) {
        delete register_pt;
    }
    if (retrieve_pt != NULL) {
        delete retrieve_pt;
    }
}

bool EndpointManager::initHandlers() {
    try {
        ping_pt = new PingEndpoint(logger, record_mgr);
        update_pt = new UpdateEndpoint(logger, record_mgr);
        register_pt = new RegisterEndpoint(logger, record_mgr);
        retrieve_pt = new RetrieveEndpoint(logger, record_mgr);
        remove_pt = new RemoveEndpoint(logger, record_mgr);
        return true;
    }
    catch(const std::exception& e) {
        logger->error("Failed to initialize Endpoint Classes");
        return false;
    }
}

bool EndpointManager::configure() {
    try {
        Rest::Routes::Post(router, "/record/create/:name", Rest::Routes::bind(&RegisterEndpoint::handle, register_pt));
        Rest::Routes::Post(router, "/ping/:name", Rest::Routes::bind(&PingEndpoint::handle, ping_pt));
        Rest::Routes::Post(router, "/record/update/:name", Rest::Routes::bind(&UpdateEndpoint::handle, update_pt));
        Rest::Routes::Post(router, "/record/remove/:name", Rest::Routes::bind(&RemoveEndpoint::handle, remove_pt));
        Rest::Routes::Get(router, "/record/:name", Rest::Routes::bind(&RetrieveEndpoint::handle, retrieve_pt));
        return true;
    }
    catch(const std::exception& e) {
        logger->error("Failed to bind API routes to class handlers");
        return false;
    }
}