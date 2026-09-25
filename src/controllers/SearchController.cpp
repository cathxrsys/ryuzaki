#include "SearchController.hpp"
#include "logger.hpp"
#include <string>

#include "sources/SourceTypes.hpp"
#include "task/TaskManager.hpp"

#include "globals.hpp"


static std::string serialize_json(const Json::Value& value) {
    Json::StreamWriterBuilder builder;
    builder["emitUTF8"] = true;
    builder["indentation"] = "";

    return Json::writeString(builder, value);
}

Task<HttpResponsePtr> SearchController::search_start(HttpRequestPtr req) {
    SPDLOG_DEBUG("GET [/search/start] Handling request");
    
    SearchQuery query;

    query.entity_name = req->getParameter("entity");

    // std::unique_ptr<INormalizer> normalizer = select_normalizer(query.entity_name);
    // query.query = normalizer->normalize(req->getParameter("query"));

    query.query = req->getParameter("query");

    if(query.entity_name.empty() || query.query.empty()) {
        co_return HttpResponse::newHttpResponse(
            k400BadRequest,
            CT_TEXT_PLAIN
        );
    }

    std::string task_id = g_task_manager->create_task(query);
    
    Json::Value response_json;
    response_json["status"] = "ok";
    response_json["task_id"] = task_id;

    co_return HttpResponse::newHttpJsonResponse(response_json);
}

Task<HttpResponsePtr> SearchController::search_results(HttpRequestPtr req, std::string taskId) {
    SPDLOG_DEBUG("GET [/search/results/{}] Handling request", taskId);

    std::optional<SearchTask> result = g_task_manager->get_task(taskId);

    if(!result.has_value()) {
        co_return HttpResponse::newHttpResponse(
            k404NotFound,
            CT_TEXT_PLAIN
        );
    }

    const SearchTask& task = *result;

    Json::Value response_json;
    response_json["task_id"] = task.id;
    response_json["status"] = static_cast<int>(task.status);

    switch(task.status) {
        case TaskStatus::Pending:
        case TaskStatus::InProgress:
            break;

        case TaskStatus::Completed: {
            Json::Value results_json(Json::arrayValue);
            for(const auto& sr : task.result.data) {
                Json::Value sr_json;
                sr_json["source"] = sr.source;
                Json::Value json_data;
                Json::Reader().parse(boost::json::serialize(sr.data), json_data);
                sr_json["data"] = json_data;
                results_json.append(sr_json);
            }
            response_json["results"] = results_json;
            break;
        }

        case TaskStatus::Failed:
            response_json["error"] = task.result.error.message;
            break;
    }

    auto response = HttpResponse::newHttpResponse();
    response->setContentTypeString("application/json; charset=utf-8");
    response->setBody(serialize_json(response_json));

    co_return response;
}