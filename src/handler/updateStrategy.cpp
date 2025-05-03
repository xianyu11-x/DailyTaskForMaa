#include "updateStrategy.h"
#include "co_async/awaiter/task.hpp"
#include "co_async/generic/allocator.hpp"
#include "co_async/utils/debug.hpp"
#include "co_async/utils/expected.hpp"
#include "conf/conf.h"
#include "levelManager/levelManager.h"
#include "mysqlConnectPool/sqlConnectPool.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "sqlMap/sqlMap.h"
#include "utils/jsonUtil.hpp"
#include "utils/timeUtil.hpp"
#include "utils/uuid.hpp"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <functional>
#include <iomanip>
#include <mysql/mysql.h>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
std::string parseSimpleStrategy(std::string& strategyStr)
{
    auto simpleStrategy = stringToJson(strategyStr);
    rapidjson::Document fullStrategy;
    fullStrategy.SetObject();
    auto allocator = fullStrategy.GetAllocator();
    for (int i = 1; i <= 7; i++) {
        rapidjson::Value strategyArray(rapidjson::kArrayType);
        rapidjson::Value task1(rapidjson::kObjectType);
        task1.AddMember("taskType", "Settings-Stage1", allocator);
        rapidjson::Value paramsArray(rapidjson::kArrayType);
        auto TaskParams = simpleStrategy[std::to_string(i).c_str()].GetArray();
        for (const auto& level : TaskParams) {
            paramsArray.PushBack(rapidjson::Value(level, allocator),
                allocator);
        }
        task1.AddMember("params", paramsArray, allocator);
        strategyArray.PushBack(task1, allocator);
        rapidjson::Value task2(rapidjson::kObjectType);
        task2.AddMember("taskType", "LinkStart", allocator);
        strategyArray.PushBack(task2, allocator);
        rapidjson::Value key(std::to_string(i).c_str(), allocator);
        fullStrategy.AddMember(key, strategyArray, allocator);
    }
    return jsonToString(fullStrategy);
}

bool checkStrategy(std::string& strategyStr)
{
    auto fullStrategy = stringToJson(strategyStr);
    for (int i = 1; i <= 7; i++) {
        std::string dayKey = std::to_string(i);
        if (!fullStrategy.HasMember(dayKey.c_str()))
            return false;
        if (!fullStrategy[dayKey.c_str()].IsArray())
            return false;
        auto oneDayStrategy = fullStrategy[dayKey.c_str()].GetArray();

        if (oneDayStrategy.Size() < 2)
            return false;
        // TODO:看看后续要不要简化判定
        const auto& task1 = oneDayStrategy[0];
        if (!task1.HasMember("taskType") || !task1["taskType"].IsString() || strcmp(task1["taskType"].GetString(), "Settings-Stage1"))
            return false;
        if (!task1.HasMember("params") || !task1["params"].IsArray())
            return false;

        const auto& task2 = oneDayStrategy[1];
        if (!task2.HasMember("taskType") || !task2["taskType"].IsString() || strcmp(task2["taskType"].GetString(), "LinkStart") != 0)
            return false;
    }
    return true;
}

Task<Expected<>> updateStrategy(HTTPServer::IO& io)
{
    auto request = co_await io.request_body();
    rapidjson::Document requestDOM;
    requestDOM.Parse(request.value().c_str());
    // TODO:合法性校验
    auto connPool = connectionPool::GetInstance();
    auto conn = co_await connPool->GetConnection();
    std::string userID = requestDOM["user"].GetString();
    std::string deviceID = requestDOM["device"].GetString();
    std::string taskStrategy = "";
    std::string oldTaskTime = "";
    std::string updateModel = requestDOM["updateModel"].GetString();
    std::string strategyModel = requestDOM["strategyModel"].GetString();
    std::string newTaskTime = "";
    std::string newTaskStrategy = "";
    if (requestDOM.HasMember("dailyTaskStrategy")) {
        taskStrategy = requestDOM["dailyTaskStrategy"].GetString();
    }
    if (requestDOM.HasMember("oldDailyTaskTime")) {
        oldTaskTime = requestDOM["oldDailyTaskTime"].GetString();
    }
    if (requestDOM.HasMember("newDailyTaskTime")) {
        newTaskTime = requestDOM["newDailyTaskTime"].GetString();
    }
    auto MAAUserInfos = queryMAAUserInfo(conn, userID, deviceID);
    if (MAAUserInfos.empty()) {
        debug(), "userInfo not exist"s;
        co_await co_await HTTPServerUtils::make_ok_response(io,
            "userInfo not exist");
        co_await connPool->ReleaseConnection(conn);
        co_return {};
    } else {
        bool fullStrategy = true;
        if (taskStrategy != "") {
            if (strategyModel == "SIMPLE") {
                fullStrategy = false;
                newTaskStrategy = parseSimpleStrategy(taskStrategy);
            } else if (checkStrategy(taskStrategy)) {
                newTaskStrategy = taskStrategy;
            } else {
                debug(), "error strategy"s;
                co_await co_await HTTPServerUtils::make_ok_response(io,
                    "error strategy");
                co_await connPool->ReleaseConnection(conn);
                co_return {};
            }
        }
        if (updateModel == "INSERT" && newTaskTime != "") {
            vector<MAADailyTaskPlan> insertList;
            MAADailyTaskPlan newPlan { .planID = generateUUID(), .userID = userID, .deviceID = deviceID, .dailyTaskStrategy = newTaskStrategy, .dailyTaskTime = newTaskTime };
            insertList.push_back(newPlan);
            auto res = insertMAADailyTaskPlan(conn, insertList);
            if (!res) {
                debug(), "insert strategy error"s;
                co_await co_await HTTPServerUtils::make_ok_response(io,
                    "insert strategy error");
                co_await connPool->ReleaseConnection(conn);
                co_return {};
            }
        } else if (updateModel == "MODIFY" && oldTaskTime != "") {
            auto userStrategyList = queryMAAUserStrategy(conn, userID, deviceID);
            for (const auto& userStrategy : userStrategyList) {
                if (userStrategy.dailyTaskTime == oldTaskTime) {
                    std::unordered_map<std::string, std::string> updateColMap;
                    if (newTaskTime != "")
                        updateColMap["dailyTaskTime"] = newTaskTime;
                    if (taskStrategy != "")
                        updateColMap["dailyTaskStrategy"] = newTaskStrategy;
                    auto res = updateMAADailyTaskPlan(conn, userStrategy.planID, userID, deviceID, updateColMap);
                    if (!res) {
                        debug(), "update strategy error"s;
                        co_await co_await HTTPServerUtils::make_ok_response(io,
                            "update strategy error");
                        co_await connPool->ReleaseConnection(conn);
                        co_return {};
                    }
                    break;
                }
            }
        } else if (updateModel == "DELETE" && oldTaskTime != "") {
            auto userStrategyList = queryMAAUserStrategy(conn, userID, deviceID);
            for (const auto& userStrategy : userStrategyList) {
                if (userStrategy.dailyTaskTime == oldTaskTime) {
                    auto res = deleteMAADailyTaskPlan(conn, userStrategy.planID);
                    if (!res) {
                        debug(), "delete strategy error"s;
                        co_await co_await HTTPServerUtils::make_ok_response(io,
                            "delete strategy error");
                        co_await connPool->ReleaseConnection(conn);
                        co_return {};
                    }
                }
            }
        }

        co_await connPool->ReleaseConnection(conn);
        co_await co_await HTTPServerUtils::make_ok_response(io, "updateStrategy");
        co_return {};
    }
}