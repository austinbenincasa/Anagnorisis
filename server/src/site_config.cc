#include <string>
#include <fstream>
#include "site_config.hpp"


SiteConfig::SiteConfig(std::string config) : config_path(config) {
    parseConfig();
}

SiteConfig::~SiteConfig() {}

void SiteConfig::parseConfig() {
    try {
        config = YAML::LoadFile(config_path);
    } 
    catch (YAML::Exception) {
        std::cout << "SiteConfig: Could not parse " << config_path << std::endl;
        valid = false;
        return;
    }
    if (config.IsNull()) {
        valid = false;
    }
    else {
        config_core = config["SiteConfig"]["core"];
        config_log = config["SiteConfig"]["log"];
    }
}

std::string SiteConfig::get_core(std::string key, std::string value) {
    if (config_core[key]) {
        return config_core[key].as<std::string>();
    }
    else {
        return value;
    }
}

int SiteConfig::get_core(std::string key, int value) {
    if (config_core[key]) {
        return config_core[key].as<int>();
    }
    else {
        return value;
    }
}

bool SiteConfig::get_core(std::string key) {
    if (config_core[key]) {
        return config_core[key].as<bool>();
    }
    else {
        return false;
    }
}

std::string SiteConfig::get_log(std::string key, std::string value) {
    if (config_log[key]) {
        return config_log[key].as<std::string>();
    }
    else {
        return value;
    }
}

int SiteConfig::get_log(std::string key, int value) {
    if (config_log[key]) {
        return config_log[key].as<int>();
    }
    else {
        return value;
    }
}
bool SiteConfig::get_log(std::string key) {
    if (config_log[key]) {
        return config_log[key];
    }
    else {
        return false;
    }
}

bool SiteConfig::isValid() {
    return valid;
}

