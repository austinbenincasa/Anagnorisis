#include "anagnorisis.hpp"
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Infos.hpp>
#include <sstream>

Anagnorisis::Anagnorisis(std::string url, int threads, int max_con, int timeout)
: url(url), threads(threads), max_con(max_con), timeout(timeout) {
    //auto pinger = std::bind(&Anagnorisis::ping, this);
    //pingThread = std::thread(pinger);
}

Anagnorisis::~Anagnorisis() {
    running = false;
    if(pingThread.joinable()) {
        pingThread.join();
    }
}

AnagnorisisResponse Anagnorisis::retrieveRecord(std::string name, std::string& record) {
    std::string request_url = url + "/record/" + name;
    rapidjson::Document json;
    std::stringstream response;
	try 
	{
		curlpp::Cleanup cleaner;
		curlpp::Easy request;
        request.setOpt(new curlpp::options::Url(request_url)); 
        request.setOpt(new curlpp::options::Verbose(false));
        request.setOpt(new curlpp::options::WriteStream(&response));
        request.perform();
	}
    catch ( curlpp::LogicError & e ) {
        return APIERROR;
    }
    catch ( curlpp::RuntimeError & e ) {
        return CLIENTERROR;
    }
    if(json.Parse(response.str().c_str()).HasParseError()) {
        return INVALIDRESP;
    }
    else {
        if(!json["retrieved"].GetBool()) {
            return (AnagnorisisResponse)json["error_code"].GetInt(); 
        }
        else {
            record = json["record"].GetString();
            return OK;
        }
    }
}

AnagnorisisResponse Anagnorisis::removeRecord(std::string name) {
    if(record_ids.count(name) == 0) {
        return NORECORDID;
    }
    std::string request_url = url + "/record/remove/" + name;
    std::stringstream response;
    rapidjson::Document json;
    rapidjson::StringBuffer strbuf;
    json.SetObject();
    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
    rapidjson::Document::AllocatorType& allocator = json.GetAllocator();
    json.AddMember("id", rapidjson::Value(record_ids[name].c_str(), allocator).Move(), allocator);
    json.Accept(writer);
    curlpp::Cleanup cleaner;
    curlpp::Easy request;
    try {
        request.setOpt(new curlpp::options::Url(request_url)); 
        request.setOpt(new curlpp::options::Verbose(false)); 
        std::list<std::string> header; 
        header.push_back("Content-Type: application/json"); 
        request.setOpt(new curlpp::options::HttpHeader(header)); 
        request.setOpt(new curlpp::options::PostFields(strbuf.GetString()));
        request.setOpt(new curlpp::options::PostFieldSize(strbuf.GetSize()));
        request.setOpt(new curlpp::options::WriteStream(&response));
        request.perform();
    }
    catch ( curlpp::LogicError & e ) {
        return APIERROR;
    }
    catch ( curlpp::RuntimeError & e ) {
        return CLIENTERROR;
    }
    if(json.Parse(response.str().c_str()).HasParseError()) {
        return INVALIDRESP;
    }
    if(!json["removed"].GetBool()) {
        return (AnagnorisisResponse)json["error_code"].GetInt();
    }
    else {
        record_ids.erase(name);
        return OK;
    }
}

AnagnorisisResponse Anagnorisis::createRecord(std::string name, std::string data) {
    if(record_ids.count(name) != 0) {
        return RECORDEXISTS;
    }
    std::string request_url = url + "/record/create/" + name;
    std::stringstream response;
    std::string id;
    rapidjson::Document json;
    rapidjson::StringBuffer strbuf;
    json.SetObject();
    rapidjson::Document::AllocatorType& allocator = json.GetAllocator();
    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
    json.AddMember("data", rapidjson::Value(data.c_str(), allocator).Move(), allocator);
    json.Accept(writer);
    curlpp::Cleanup cleaner;
    curlpp::Easy request;
    try {
        request.setOpt(new curlpp::options::Url(request_url)); 
        request.setOpt(new curlpp::options::Verbose(false)); 
        std::list<std::string> header; 
        header.push_back("Content-Type: application/json"); 
        request.setOpt(new curlpp::options::HttpHeader(header)); 
        request.setOpt(new curlpp::options::PostFields(strbuf.GetString()));
        request.setOpt(new curlpp::options::PostFieldSize(strbuf.GetSize()));
        request.setOpt(new curlpp::options::WriteStream(&response));
        request.perform();
    }
    catch ( curlpp::LogicError & e ) {
        return APIERROR;
    }
    catch ( curlpp::RuntimeError & e ) {
        return CLIENTERROR;
    }
    if(json.Parse(response.str().c_str()).HasParseError()) {
        return INVALIDRESP;
    }
    if(!json["created"].GetBool()) {
        return (AnagnorisisResponse)json["error_code"].GetInt();
    }
    else {
        record_ids[name] = json["id"].GetString();
        return OK;
    }
}

AnagnorisisResponse Anagnorisis::updateRecord(std::string name, std::string data) {
    if(record_ids.count(name) == 0) {
        return NORECORDID;
    }
    std::string request_url = url + "/record/update/" + name;
    std::stringstream response;
    rapidjson::Document json;
    rapidjson::StringBuffer strbuf;
    json.SetObject();
    rapidjson::Document::AllocatorType& allocator = json.GetAllocator();
    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
    json.AddMember("id", rapidjson::Value(record_ids[name].c_str(), allocator).Move(), allocator);
    json.AddMember("data", rapidjson::Value(data.c_str(), allocator).Move(), allocator);
    json.Accept(writer);
    try {
        curlpp::Cleanup cleaner;
        curlpp::Easy request;
        request.setOpt(new curlpp::options::Url(request_url)); 
        request.setOpt(new curlpp::options::Verbose(false)); 
        std::list<std::string> header; 
        header.push_back("Content-Type: application/json"); 
        request.setOpt(new curlpp::options::HttpHeader(header)); 
        request.setOpt(new curlpp::options::PostFields(strbuf.GetString()));
        request.setOpt(new curlpp::options::PostFieldSize(strbuf.GetSize()));
        request.setOpt(new curlpp::options::WriteStream(&response));
        request.perform();
    }
    catch ( curlpp::LogicError & e ) {
        return APIERROR;
    }
    catch ( curlpp::RuntimeError & e ) {
        return CLIENTERROR;
    }
    if(json.Parse(response.str().c_str()).HasParseError()) {
        return INVALIDRESP;
    }
    if(!json["updated"].GetBool()) {
        return (AnagnorisisResponse)json["error_code"].GetInt();
    }
    else {
        return OK;
    }
}

std::string Anagnorisis::getAnagnorisisResponse(AnagnorisisResponse response) {
    switch (response)
    {
        case OK:
            return "Operation successfuly";
        case IDERROR:
            return "Id is not valid";
        case IDMISMATCH:
            return "Id is not associated with a record";
        case NAMEERROR:
            return "Name is already associated to a record";
        case NORECORD:
            return "No record found";
        case RECORDEXISTS:
            return "Name is already associated to a record";
        case NORECORDID:
            return "Record ID not found: cannot complete operation";
        case INVALIDRESP:
            return "Server returned a invalid response";
        case INVALIDREQ:
            return "API sent invalid request";  
        case SSLERROR:
            return "SSL Error occured";
        case CLIENTERROR:
            return "Error occured preforming request";
        case APIERROR:
            return "Error occured in API, this is a bug";
        default:
            return "Unknown Error";
    }
}

void Anagnorisis::ping() {
    std::string request_url;
    std::stringstream response;
    std::map<std::string, std::string>::iterator it;
    rapidjson::Document json;
    rapidjson::StringBuffer strbuf;
    json.SetObject();
    rapidjson::Document::AllocatorType& allocator = json.GetAllocator();
    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
    while(running) {
        if(record_ids.size() > 0) {
            for ( it = record_ids.begin(); it != record_ids.end(); it++ ) {
                request_url = url + "/ping/" + it->first;
                json.AddMember("id", rapidjson::Value(it->second.c_str(), allocator).Move(), allocator);
                json.Accept(writer);
                try {
                    curlpp::Cleanup cleaner;
                    curlpp::Easy request;
                    request.setOpt(new curlpp::options::Url(request_url)); 
                    request.setOpt(new curlpp::options::Verbose(false)); 
                    std::list<std::string> header; 
                    header.push_back("Content-Type: application/json"); 
                    request.setOpt(new curlpp::options::HttpHeader(header)); 
                    request.setOpt(new curlpp::options::PostFields(strbuf.GetString()));
                    request.setOpt(new curlpp::options::PostFieldSize(strbuf.GetSize()));
                    request.setOpt(new curlpp::options::WriteStream(&response));
                    request.perform();
                }
                catch ( curlpp::LogicError & e ) {
                    std::cout << e.what() << std::endl;
                }
                catch ( curlpp::RuntimeError & e ) {
                    std::cout << e.what() << std::endl;
                }
                if(!json.Parse(response.str().c_str()).HasParseError()) {
                    //if ping failed remove
                    if(!json["pinged"].GetBool()) {
                        record_ids.erase(it->first);
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
}