#include "remove.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"

RemoveEndpoint::RemoveEndpoint(std::shared_ptr<spdlog::logger> logger, RecordManager* record_mgr) 
: logger(logger), record_mgr(record_mgr) {

}

RemoveEndpoint::~RemoveEndpoint() {
    
}

void RemoveEndpoint::handle(const Request& request, ResponseWriter response) {
    auto header = request.headers();
    RecordResponse resp;
    std::string id;
    bool error = false;
    rapidjson::Document jsonreq;
    rapidjson::Document jsonresp;
    rapidjson::StringBuffer strbuf;
    rapidjson::Document::AllocatorType& allocator = jsonresp.GetAllocator();
    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
    response.headers().add<ContentType>(MIME(Application, Json));
    jsonresp.SetObject();
    jsonresp.AddMember("version", 1, allocator);
    if(!request.hasParam(":name")) {
        logger->error("no name");   
        jsonresp.AddMember("id", "", allocator);
        jsonresp.AddMember("name", "", allocator);
        jsonresp.AddMember("removed", rapidjson::Value(false), allocator);
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
            logger->info(request.body().c_str());
            jsonresp.AddMember("id", "", allocator);
            jsonresp.AddMember("updated", rapidjson::Value(false), allocator);
            jsonresp.AddMember("error", "Invalid JSON", allocator);
            jsonresp.AddMember("error_code", 9, allocator);
            jsonresp.Accept(writer);
            response.send(Pistache::Http::Code::Bad_Request, strbuf.GetString());
        }
        else {
            resp = record_mgr->removeRecord(id, name);
            jsonresp.AddMember("id", rapidjson::Value(id.c_str(), allocator).Move(), allocator);
            if(resp == OK) {
                jsonresp.AddMember("error", "", allocator);
                jsonresp.AddMember("error_code", 0, allocator);
                jsonresp.AddMember("removed", rapidjson::Value(true), allocator);
                jsonresp.Accept(writer);
                response.send(Pistache::Http::Code::Ok, strbuf.GetString());
            }
            else {
                jsonresp.AddMember("removed", rapidjson::Value(false), allocator);
                jsonresp.AddMember("error_code", record_mgr->getErrorCode(resp), allocator);
                jsonresp.AddMember("error", rapidjson::Value(record_mgr->getResponseMsg(resp).c_str(), allocator).Move(), allocator);
                jsonresp.Accept(writer);
                response.send(Pistache::Http::Code::Bad_Request, strbuf.GetString());
            }
        }
    }
}