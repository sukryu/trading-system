#include <catch2/catch.hpp>
#include <trantor/net/EventLoopThread.h>
#include <drogon/drogon.h>
#include <drogon/HttpClient.h>
#include <future>
#include <thread>
#include <chrono>

// 전역 초기화 전용(테스트 시작 전에 이벤트 루프 기동)
struct DrogonEventLoop {
    DrogonEventLoop() {
        // 서버 없이 단순 이벤트 루프 실행
        // Drogon을 run하지 않고도 이벤트 루프를 구동하려면 
        // 별도 trantor::EventLoopThread를 사용할 수도 있음.
        
        // 여기서는 HttpClient 콜백 처리를 위해 내부적으로 사용되는 이벤트 루프를 돌리기 위해 
        // drogon::app()을 run할 필요가 있음. 하지만 run은 블록함.
        // 대안: trantor::EventLoopThread를 하나 만들어 HttpClient용으로 사용.
        loopThread_.run();
    }

    ~DrogonEventLoop() {
        loopThread_.getLoop()->quit();
    }

    trantor::EventLoopThread loopThread_;
};

static DrogonEventLoop eventLoopEnv; // 전역 이벤트 루프 준비

TEST_CASE("HealthController Integration Test", "[controllers][integration]") {
    // 여기서는 HttpClient를 trantor::EventLoopThread를 이용해 구동한다.
    auto loop = eventLoopEnv.loopThread_.getLoop();
    auto client = drogon::HttpClient::newHttpClient("http://127.0.0.1:8000", loop);
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Get);
    req->setPath("/health");

    std::promise<bool> testPromise;
    auto testFuture = testPromise.get_future();

    client->sendRequest(req, [&testPromise](drogon::ReqResult result,
                                            const drogon::HttpResponsePtr& resp) {
        bool success = false;
        try {
            REQUIRE(result == drogon::ReqResult::Ok);
            REQUIRE(resp != nullptr);
            REQUIRE(resp->getStatusCode() == drogon::k200OK);
            auto jsonBody = resp->getJsonObject();
            REQUIRE(jsonBody != nullptr);
            REQUIRE((*jsonBody).isMember("status"));
            REQUIRE((*jsonBody)["status"].asString() == "OK");
            REQUIRE((*jsonBody).isMember("timestamp"));
            REQUIRE((*jsonBody)["timestamp"].isInt64());
            success = true;
        } catch (...) {
            // 실패시 success=false
        }
        testPromise.set_value(success);
    });

    // 콜백 실행을 기다리기 위해
    auto status = testFuture.wait_for(std::chrono::seconds(5));
    REQUIRE(status == std::future_status::ready);
    REQUIRE(testFuture.get() == true);
}
