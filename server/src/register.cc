#include "register.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include <exception>

RegisterEndpoint::RegisterEndpoint(std::shared_ptr<spdlog::logger> logger, RecordManager* record_mgr) 
: logger(logger), record_mgr(record_mgr) {

}

RegisterEndpoint::~RegisterEndpoint() {
    
}

void RegisterEndpoint::handle(const Request& request, ResponseWriter response) {
    auto header = request.headers();
    bool error = false;
    std::string id;
    std::string data;
    RecordResponse resp;
    rapidjson::Document jsonreq;
    rapidjson::Document jsonresp;
    rapidjson::StringBuffer strbuf;
    jsonresp.SetObject();
    rapidjson::Document::AllocatorType& allocator = jsonresp.GetAllocator();
    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
    response.headers().add<ContentType>(MIME(Application, Json));
    jsonresp.AddMember("version", 1, allocator);
    if(!request.hasParam(":name")) {
        jsonresp.AddMember("name", "", allocator);
        jsonresp.AddMember("id", "", allocator);
        jsonresp.AddMember("error", "No param 'name'", allocator);
        jsonresp.AddMember("error_code", 9, allocator);
        jsonresp.AddMember("created", rapidjson::Value(false), allocator);
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
            if(jsonreq.HasMember("data") && jsonreq["data"].IsString()) {
                data = jsonreq["data"].GetString();
            }
            else {
                error = true;
            }
        }
        if(error) {
            jsonresp.AddMember("id", "", allocator);
            jsonresp.AddMember("error", "Invalid JSON", allocator);
            jsonresp.AddMember("created", rapidjson::Value(false), allocator);
            jsonresp.AddMember("error_code", 9, allocator);
            response.send(Pistache::Http::Code::Bad_Request, strbuf.GetString());
        }
        else {
            id = record_mgr->generateUUID(36);
            resp = record_mgr->createRecord(id, name, data);
            if(resp == OK) {
                jsonresp.AddMember("id", rapidjson::Value(id.c_str(), allocator).Move(), allocator);
                jsonresp.AddMember("error", "", allocator);
                jsonresp.AddMember("error_code", 0, allocator);
                jsonresp.AddMember("created", rapidjson::Value(true), allocator);
                jsonresp.Accept(writer);
                response.send(Pistache::Http::Code::Ok, strbuf.GetString());
            }
            else {
                jsonresp.AddMember("id", "", allocator);
                jsonresp.AddMember("error", rapidjson::Value(record_mgr->getResponseMsg(resp).c_str(), allocator).Move(), allocator);
                jsonresp.AddMember("created", rapidjson::Value(false), allocator);
                jsonresp.AddMember("error_code", resp, allocator);
                jsonresp.Accept(writer);
                response.send(Pistache::Http::Code::Bad_Request, strbuf.GetString());
            }
        }
    }
}