#include "PingController.hpp"
#include "logger.hpp"
#include <string>


static Task<std::string> getPingString() {
    co_return "pong";
}

static Task<std::string> getKiraString() {
    co_return "kira";
}

static Task<std::string> getLString() {
    co_return "L";
}

Task<HttpResponsePtr> PingController::ping(HttpRequestPtr req) {
    SPDLOG_DEBUG("GET [/ping] Handling request");
    
    std::string data = co_await getPingString();
    
    auto resp = HttpResponse::newHttpResponse();
    resp->setContentTypeCode(CT_TEXT_PLAIN);
    resp->setBody(data);
    
    co_return resp;
}

Task<HttpResponsePtr> PingController::l(HttpRequestPtr req) {
    SPDLOG_DEBUG("GET [/l] Handling request");
    
    std::string data = co_await getKiraString();
    
    auto resp = HttpResponse::newHttpResponse();
    resp->setContentTypeCode(CT_TEXT_PLAIN);
    resp->setBody(data);
    
    co_return resp;
}

Task<HttpResponsePtr> PingController::kira(HttpRequestPtr req) {
    SPDLOG_DEBUG("GET [/kira] Handling request");
    
    std::string data = co_await getLString();
    
    auto resp = HttpResponse::newHttpResponse();
    resp->setContentTypeCode(CT_TEXT_PLAIN);
    resp->setBody(data);
    
    co_return resp;
}