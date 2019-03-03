#include <string>
#include <iostream>
#include <chrono>
#include "service.hpp"


int main(int argc, char const *argv[])
{
    Service * service;
    bool interrupted = false;
    std::string config_path = "../config.yml";
    if (argc == 2) {
        config_path = std::string(argv[1]);
    }
    service = new Service(config_path);
    service->start();
    while(!interrupted) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        switch(service->getState()) {
            case starting:
                interrupted = false;
                break;
            case running:
                interrupted = false;
                break;
            case stopping:
                interrupted = true;
                break;
            case stopped:
                interrupted = true;
                break;
            case restarting:
                interrupted = false;
                break;
            case crashed:
                interrupted = true;
                break;
            default:
                interrupted = true;
        }
    }
    service->cleanup();
    delete service;
    std::cout << "Anagnorisis Service has stopped" << std::endl;
    return 0;
}
