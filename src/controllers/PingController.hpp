#pragma once

#include <drogon/HttpController.h>
#include <drogon/utils/coroutine.h>

using namespace drogon;

class PingController : public HttpController<PingController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(PingController::ping, "/ping", Get);
    ADD_METHOD_TO(PingController::l, "/l", Get);
    ADD_METHOD_TO(PingController::kira, "/kira", Get);
    METHOD_LIST_END

    Task<HttpResponsePtr> ping(HttpRequestPtr req);
    Task<HttpResponsePtr> l(HttpRequestPtr req);
    Task<HttpResponsePtr> kira(HttpRequestPtr req);
};