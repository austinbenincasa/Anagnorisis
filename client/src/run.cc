#include "anagnorisis.hpp"
#include <iostream>
#include <chrono>

int main(int argc, char const *argv[])
{
    AnagnorisisResponse response;
    Anagnorisis* client;
    std::string record;
    client = new Anagnorisis("http://localhost:8081", 1, 10, 10);
    for(int i = 0; i < 1000; i++) {
        response = client->createRecord(std::to_string(i), std::string("test" + std::to_string(i)));
        if(response != OK) {
            std::cout << "Error: " << client->getAnagnorisisResponse(response) << std::endl;
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(4000));
    for(int i = 0; i < 1000; i++) {
        response = client->retrieveRecord(std::to_string(i), record);
        if(response != OK) {
            std::cout << "Error: " << response << std::endl;
            std::cout << "Error: " << client->getAnagnorisisResponse(response) << std::endl;
        }
        else {
            std::cout << record << std::endl;
        }
    }
    /*
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    response = client->updateRecord("austin", "test2");
    if(response == OK) {
        std::cout << "updated" << std::endl;
    }
    else {
        std::cout << "Error: " << response << std::endl;
        std::cout << "Error: " << client->getAnagnorisisResponse(response) << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    response = client->retrieveRecord("austin", record);
    if(response == OK) {
        std::cout << record << std::endl;
    }
    else {
        std::cout << "Error: " << response << std::endl;
        std::cout << "Error: " << client->getAnagnorisisResponse(response) << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    response = client->removeRecord("austin");
    if(response == OK) {
        std::cout << "removed" << std::endl;
    }
    else {
        std::cout << "Error: " << response << std::endl;
        std::cout << "Error: " << client->getAnagnorisisResponse(response) << std::endl;
    }
    */
}