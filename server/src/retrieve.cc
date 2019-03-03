#include "retrieve.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"

RetrieveEndpoint::RetrieveEndpoint(std::shared_ptr<spdlog::logger> logger, RecordManager* record_mgr) 
: logger(logger), record_mgr(record_mgr) {

}

RetrieveEndpoint::~RetrieveEndpoint() {
    
}

void RetrieveEndpoint::handle(const Request& request, ResponseWriter response) {
    auto header = request.headers();
    RecordResponse resp;
    std::string data;
    rapidjson::Document jsonresp;
    rapidjson::StringBuffer strbuf;
    rapidjson::Document::AllocatorType& allocator = jsonresp.GetAllocator();
    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
    response.headers().add<ContentType>(MIME(Application, Json));
    jsonresp.SetObject();
    jsonresp.AddMember("version", 1, allocator);
    if(!request.hasParam(":name")) {
        jsonresp.AddMember("name", "", allocator);
        jsonresp.AddMember("record", "", allocator);
        jsonresp.AddMember("retrieved", rapidjson::Value(false), allocator);
        jsonresp.AddMember("error", "No param 'name'", allocator);
        jsonresp.AddMember("error_code", 9, allocator);
        jsonresp.Accept(writer);
        response.send(Pistache::Http::Code::Bad_Request, strbuf.GetString());
    }
    else {
        auto name = request.param(":name").as<std::string>();
        jsonresp.AddMember("name", rapidjson::Value(name.c_str(), allocator).Move(), allocator);
        resp = record_mgr->retrieveRecord(name, data);
        if(resp == OK) {
            jsonresp.AddMember("record", rapidjson::Value(data.c_str(), allocator).Move(), allocator);            
            jsonresp.AddMember("error", "", allocator);
            jsonresp.AddMember("error_code", 0, allocator);
            jsonresp.AddMember("retrieved", rapidjson::Value(true), allocator);
            jsonresp.Accept(writer);
            response.send(Pistache::Http::Code::Ok, strbuf.GetString());
        }
        else {
            jsonresp.AddMember("record", "", allocator);
            jsonresp.AddMember("retrieved", rapidjson::Value(false), allocator);
            jsonresp.AddMember("error", rapidjson::Value(record_mgr->getResponseMsg(resp).c_str(), allocator).Move(), allocator);
            jsonresp.AddMember("error_code", record_mgr->getErrorCode(resp), allocator);
            jsonresp.Accept(writer);
            response.send(Pistache::Http::Code::Bad_Request, strbuf.GetString());
        }
    }
}