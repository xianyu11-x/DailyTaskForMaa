#include "router.h"
#include "handler/getTask.h"
#include "handler/reportStatus.h"
#include "handler/updateLevel.h"
#include "handler/updateStrategy.h"
#include "handler/quickTask.h"
#include "handler/getStrategy.h"
#include "co_async/net/http_server.hpp"


void setAllRoute(HTTPServer &server) {
    server.route("GET", "/index", [](HTTPServer::IO &io) -> Task<Expected<>> {
        co_await co_await HTTPServerUtils::make_ok_response(io, "<h1>It works!</h1>");
        co_return {};
    });
    server.route("POST", "/maa/getTask",getTask);
    server.route("POST", "/maa/reportStatus",reportStatus);
    server.route("POST", "/maa/updateSideStory",updateLevel);
    server.route("POST", "/maa/updateStrategy",updateStrategy);
    server.route("POST", "/maa/quickTask",quickTask);
    server.route("GET","/maa/getStrategy", getStrategy);
    #if CO_ASYNC_DEBUG
    //server.route("GET", "/maa/getTask",getTask);
    //server.route("GET", "/maa/reportStatus",reportStatus);
    #endif
}