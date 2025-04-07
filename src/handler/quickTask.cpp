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
#include "quickTask.h"

Task<Expected<>> quickTask(HTTPServer::IO& io)
{
    auto connPool = connectionPool::GetInstance();
    auto conn = co_await connPool->GetConnection();
    auto request = co_await io.request_body();
    rapidjson::Document requestDOM;
    requestDOM.Parse(request.value().c_str());
    auto userID = requestDOM["user"].GetString();
    auto deviceID = requestDOM["device"].GetString();
    auto taskActions = requestDOM["taskActions"].GetArray();
    auto storeUserInfos = queryMAAUserInfo(conn, userID, deviceID);
    if (storeUserInfos.empty()) {
        debug(), "userInfo not exist"s;
        co_await co_await HTTPServerUtils::make_ok_response(io,
            "userInfo not exist");
    } else {
        auto& allocator = requestDOM.GetAllocator();
        auto quickTaskId = generateUUID();
        vector<MAAAction> taskActionsList;
        for (auto& taskAction : taskActions) {
            std::string actionId = generateUUID();
            rapidjson::Value taskIDValue(actionId.c_str(), allocator);
            taskAction.AddMember("id", taskIDValue, allocator);
            taskActionsList.push_back(MAAAction { .taskID = quickTaskId,
                .actionID = actionId,
                .actionIsFinish = "0" });
        }
        auto quickTaskStr = jsonToString(taskActions);
        vector<MAAQuickTask> quickTaskList;
        auto defaultStartTime = DateAdd(getNowTm(), -24 * 60 * 60);
        quickTaskList.push_back(MAAQuickTask { .taskID = quickTaskId,
            .userID = userID,
            .deviceID = deviceID,
            .taskCommitTime = tmToString(getNowTm(), "%Y-%m-%d %H:%M:%S"),
            .taskStartTime = tmToString(defaultStartTime, "%Y-%m-%d %H:%M:%S"),
            .taskIsFinish = "0",
            .taskActions = quickTaskStr });
        int res = insertMAAQuickTask(conn, quickTaskList);
        if (res == -1) {
            debug(), "insert quickTask error"s;
        }
        res = insertMAAAction(conn, taskActionsList);
        if (res == -1) {
            debug(), "insert action error"s;
        }
        co_await co_await HTTPServerUtils::make_ok_response(io, "quickTask");
    }
    co_await connPool->ReleaseConnection(conn);
    co_return {};
}