#include "router.h"
#include "../handler/handler.h"


void setAllRoute(HTTPServer &server) {
    server.route("GET", "/index", [](HTTPServer::IO &io) -> Task<Expected<>> {
        co_await co_await HTTPServerUtils::make_ok_response(io, "<h1>It works!</h1>");
        co_return {};
    });
    server.route("POST", "/maa/getTask",getTask);
    server.route("POST", "/maa/reportStatus",reportStatus);
    
    #if CO_ASYNC_DEBUG
    //server.route("GET", "/maa/getTask",getTask);
    //server.route("GET", "/maa/reportStatus",reportStatus);
    #endif
}