#include "conf/conf.h"
#include "levelManager/levelManager.h"
#include "mysqlConnectPool/sqlConnectPool.h"
#include "sqlMap/sqlMap.h"
#include "utils/jsonUtil.hpp"
#include "utils/timeUtil.hpp"
#include "utils/uuid.hpp"
#include "co_async/awaiter/task.hpp"
#include "co_async/generic/allocator.hpp"
#include "co_async/utils/debug.hpp"
#include "co_async/utils/expected.hpp"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <functional>
#include <iomanip>
#include <mysql/mysql.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "updateStrategy.h"


Task<Expected<>> updateStrategy(HTTPServer::IO& io)
{
    auto request = co_await io.request_body();
    rapidjson::Document requestDOM;
    requestDOM.Parse(request.value().c_str());
    // TODO:合法性校验
    auto connPool = connectionPool::GetInstance();
    auto conn = co_await connPool->GetConnection();
    auto userID = requestDOM["user"].GetString();
    auto deviceID = requestDOM["device"].GetString();
    auto taskStrategy = requestDOM["dailyTaskStrategy"].GetString();
    auto oldTaskTime = requestDOM["oldDailyTaskTime"].GetString();
    std::string newTaskTime = "";
    if (requestDOM.HasMember("newDailyTaskTime")) {
        newTaskTime = requestDOM["newDailyTaskTime"].GetString();
    }
    auto storeMAAInfos = queryMAAUserStrategy(conn, userID, deviceID);
    if (storeMAAInfos.empty()) {
        debug(), "userInfo not exist"s;
        co_await co_await HTTPServerUtils::make_ok_response(io,
            "userInfo not exist");
        co_await connPool->ReleaseConnection(conn);
        co_return {};
    } else {
        bool hasTaskTime = false;
        MAADailyTaskPlan updateMAAPlan;
        for (const auto& storeMAAPlan : storeMAAInfos) {
            if (storeMAAPlan.dailyTaskTime == oldTaskTime) {
                hasTaskTime = true;
                updateMAAPlan = storeMAAPlan;
                break;
            }
        }
        if (!hasTaskTime) {
            co_await co_await HTTPServerUtils::make_ok_response(io,
                "taskTime not exist");
            co_await connPool->ReleaseConnection(conn);
            co_return {};
        } else {
            std::unordered_map<std::string, std::string> updateColMap;
            updateColMap["dailyTaskStrategy"] = taskStrategy;
            if (newTaskTime != "") {
                updateColMap["dailyTaskTime"] = newTaskTime;
            }
            bool updateRes = updateMAADailyTaskPlan(
                conn, updateMAAPlan.planID, updateMAAPlan.userID,
                updateMAAPlan.deviceID, updateColMap);
            if (!updateRes) {
                debug(), "update task error"s;
                co_await co_await HTTPServerUtils::make_ok_response(
                    io, "update task error");
                co_await connPool->ReleaseConnection(conn);
                co_return {};
            }
        }
        co_await connPool->ReleaseConnection(conn);
        co_await co_await HTTPServerUtils::make_ok_response(io, "updateStrategy");
        co_return {};
    }
}