#include "mysqlConnectPool/sqlConnectPool.h"
#include "sqlMap/sqlMap.h"
#include "utils/jsonUtil.hpp"
#include "co_async/awaiter/task.hpp"
#include "co_async/generic/allocator.hpp"
#include "co_async/utils/debug.hpp"
#include "co_async/utils/expected.hpp"
#include "rapidjson/document.h"
#include <ctime>
#include <mysql/mysql.h>
#include <string>
#include <vector>
#include "getStrategy.h"


Task<Expected<>> getStrategy(HTTPServer::IO& io)
{
    auto connPool = connectionPool::GetInstance();
    auto conn = co_await connPool->GetConnection();
    auto request = co_await io.request_body();
    rapidjson::Document requestDOM;
    rapidjson::Document responseDOM;
    requestDOM.Parse(request.value().c_str());
    auto userID = requestDOM["user"].GetString();
    auto deviceID = requestDOM["device"].GetString();
    auto storeUserInfoVec = queryMAAUserStrategy(conn, userID, deviceID);
    if (storeUserInfoVec.empty()) {
        debug(), "userInfo not exist"s;
        co_await co_await HTTPServerUtils::make_ok_response(io,
            "userInfo not exist");
    } else {
        responseDOM.SetObject();
        auto& allocator = responseDOM.GetAllocator();
        rapidjson::Value strategyArray(rapidjson::kArrayType);
        for (const auto& storeUserInfo : storeUserInfoVec) {
            rapidjson::Value strategyInfo(rapidjson::kObjectType);
            rapidjson::Value dailyTaskTime(storeUserInfo.dailyTaskTime.c_str(),
                allocator);
            rapidjson::Value strategyStr(storeUserInfo.dailyTaskStrategy.c_str(),
                allocator);
            strategyInfo.AddMember("dailyTaskTime", dailyTaskTime, allocator);
            strategyInfo.AddMember("strategy", strategyStr, allocator);
            strategyArray.PushBack(strategyInfo, allocator);
        }
        responseDOM.AddMember("strategies", strategyArray, allocator);
        auto responseStr = jsonToString(responseDOM);
        co_await co_await HTTPServerUtils::make_ok_response(io, responseStr);
    }
    co_await connPool->ReleaseConnection(conn);
    co_return {};
}