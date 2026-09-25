#pragma once

#include <drogon/HttpController.h>
#include <drogon/utils/coroutine.h>

using namespace drogon;

class SearchController : public HttpController<SearchController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(SearchController::search_start, "/search/start", Get);
    ADD_METHOD_TO(SearchController::search_results, "/search/results/{taskId}", Get);
    METHOD_LIST_END

    Task<HttpResponsePtr> search_start(HttpRequestPtr req);
    Task<HttpResponsePtr> search_results(HttpRequestPtr req, std::string taskId);
};