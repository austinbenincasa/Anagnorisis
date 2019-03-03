#include "ping.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"

PingEndpoint::PingEndpoint(std::shared_ptr<spdlog::logger> logger, RecordManager* record_mgr) 
: logger(logger), record_mgr(record_mgr) {

}

PingEndpoint::~PingEndpoint() {
    
}

void PingEndpoint::handle(const Request& request, ResponseWriter response) {
    auto header = request.headers();
    bool error = false;
    std::string id;
    rapidjson::Document jsonreq;
    rapidjson::Document jsonresp;
    rapidjson::StringBuffer strbuf;
    jsonresp.SetObject();
    rapidjson::Document::AllocatorType& allocator = jsonresp.GetAllocator();
    response.headers().add<ContentType>(MIME(Application, Json));
    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
    jsonresp.AddMember("version", 1, allocator);
    if(!request.hasParam(":name")) {
        jsonresp.AddMember("id", "", allocator);
        jsonresp.AddMember("name", "", allocator);
        jsonresp.AddMember("pinged", rapidjson::Value(false), allocator);
        jsonresp.AddMember("error", "No param 'name'", allocator);
        jsonresp.AddMember("error_code", 9, allocator);
        jsonresp.Accept(writer);
        response.send(Pistache::Http::Code::Bad_Request, strbuf.GetString());
    }
    else {
        auto name = request.param(":name").as<std::string>();
        jsonresp.AddMember("name", rapidjson::Value(name.c_str(), allocator).Move(), allocator);
        if(jsonreq.Parse(request.body().c_str()).HasParseError()) {
            error = true;
        }
        else {
            if(jsonreq.HasMember("id") && jsonreq["id"].IsString()) {
                id = jsonreq["id"].GetString();
            }
            else {
                error = true;
            }
        }
        if(error) {
            jsonresp.AddMember("id", "", allocator);
            jsonresp.AddMember("pinged", rapidjson::Value(false), allocator);
            jsonresp.AddMember("error", "Invalid JSON", allocator);
            jsonresp.AddMember("error_code", 9, allocator);
            jsonresp.Accept(writer);
            response.send(Pistache::Http::Code::Bad_Request, strbuf.GetString());
            return;
        }
        jsonresp.AddMember("id", rapidjson::Value(id.c_str(), allocator).Move(), allocator);
        jsonresp.AddMember("name", rapidjson::Value(name.c_str(), allocator).Move(), allocator);
        jsonresp.AddMember("pinged", rapidjson::Value(true), allocator);
        jsonresp.AddMember("error", "", allocator);
        jsonresp.AddMember("error_code", 0, allocator);
        jsonresp.Accept(writer);
        response.send(Pistache::Http::Code::Ok, strbuf.GetString());
    }
}