#pragma once
#include <string>
#include <map>
#include <yaml-cpp/yaml.h>

class SiteConfig {
    public:
        SiteConfig(std::string);
        ~SiteConfig();
        bool isValid();
        std::string get_core(std::string, std::string);
        int get_core(std::string, int);
        bool get_core(std::string);
        std::string get_log(std::string, std::string);
        int get_log(std::string, int);
        bool get_log(std::string);
    private:
        void parseConfig();
        YAML::Node config;
        YAML::Node config_core;
        YAML::Node config_log;
        std::string config_path;
        bool valid = true;
};