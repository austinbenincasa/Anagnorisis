#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include "service.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"

Service::Service(std::string config_path) : config_path(config_path), start_time(time(0)) {}

void Service::stop() {
    state = stopping;
    logger->info("Stopping service");
    endpoint_mgr->shutdown();
    record_mgr->shutdown();
}

void Service::start() {
    state = starting;
    config = new SiteConfig(config_path);
    if(!config->isValid()) {
        state = crashed;
        return;
    }
    init_logging();
    logger->info("Starting service");
    if(!init_classes()) {
        state = crashed;
        return;
    }
    if(state == starting) {
        run();
    }
    else {
        stop();
    }
}

bool Service::init_classes() {
    try {
        record_mgr = new RecordManager(logger, config);
        endpoint_mgr = new EndpointManager(logger, config, record_mgr);
    }
    catch(const std::exception& e) {
        logger->error(e.what());
        return false;
    }
    return true;
}

void Service::restart() {
    state = restarting;
    logger->info("Starting service");
    endpoint_mgr->shutdown();
    start();
}

void Service::init_logging() {
    if(config->get_log("console")) {
        logger = spdlog::stdout_color_mt("console");
    }
    else {
        logger = spdlog::basic_logger_mt("Anagnorisis Logger", 
                                         config->get_log("path", "anagnorisis.log"));
        spdlog::flush_every(std::chrono::seconds(config->get_log("flush_period", 3)));
    }
    logger->info("Logger has started");
}

void Service::cleanup() {
    if(endpoint_mgr != NULL) {
        delete endpoint_mgr;
    }
    if(record_mgr != NULL) {
        delete record_mgr;
    }
    if(config != NULL) {
        delete config;
    }
    spdlog::shutdown();
}

ServiceState Service::getState() {
    return state;
}

void Service::run() {
    state = running;
    if(!endpoint_mgr->initialize()) {
        logger->error("EndpointManager did initialize, stopping service");
        stop();
    }
    if(!endpoint_mgr->start()) {
        logger->error("EndpointManager did not start, stopping service");
        stop();
    }
    else {
        record_mgr->start();
    }
}