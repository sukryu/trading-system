#include "controllers/HealthController.h"
#include <json/json.h>
#include <drogon/HttpResponse.h>
#include <ctime>

namespace controllers {

void HealthController::health(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback) 
{
    Json::Value result;
    result["status"] = "OK";
    result["timestamp"] = std::time(nullptr);
    
    auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
    callback(resp);
}

}