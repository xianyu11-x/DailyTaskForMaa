#include "mysqlConnectPool/sqlConnectPool.h"
#include "sqlMap/sqlMap.h"
#include "utils/timeUtil.hpp"
#include "co_async/awaiter/task.hpp"
#include "co_async/utils/debug.hpp"
#include "co_async/utils/expected.hpp"
#include "rapidjson/document.h"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <mysql/mysql.h>
#include <string>
#include <unordered_map>
#include <vector>
#include "reportStatus.h"


Task<Expected<>> reportStatus(HTTPServer::IO& io)
{
    auto connPool = connectionPool::GetInstance();
    auto conn = co_await connPool->GetConnection();

    auto request = co_await io.request_body();
    rapidjson::Document requestDOM;
    rapidjson::Document responseDOM;
    requestDOM.Parse(request.value().c_str());
    std::string userID = requestDOM["user"].GetString();
    std::string deviceID = requestDOM["device"].GetString();
    std::string returnTaskID = requestDOM["task"].GetString();
    auto storeUserInfos = queryMAAUserTaskStatus(conn, userID, deviceID);
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm tmNow = *std::localtime(&nowTime);
    int curSecond = tmNow.tm_hour * 3600 + tmNow.tm_min * 60 + tmNow.tm_sec;
    if (storeUserInfos.empty() || storeUserInfos.size() > 1) {
        debug(), "userInfo not exist"s;
    } else {
        auto storeUserInfo = storeUserInfos[0];
        if (storeUserInfo.dailyTaskID == returnTaskID) {
            auto MAAPlans = queryMAAUserStrategy(conn, userID, deviceID);
            if (MAAPlans.empty()) {
                debug(), "userInfo not exist"s;
            } else {
                for (auto& MAAPlan : MAAPlans) {
                    auto dailyTaskTimeTm = stringToTm(MAAPlan.dailyTaskTime, "%H:%M:%S");
                    MAAPlan.taskSeconds = (dailyTaskTimeTm.tm_hour * 3600 + dailyTaskTimeTm.tm_min * 60 + dailyTaskTimeTm.tm_sec);
                }
                std::sort(MAAPlans.begin(), MAAPlans.end(),
                    [](const MAADailyTaskPlan& a, const MAADailyTaskPlan& b) {
                        return a.taskSeconds < b.taskSeconds;
                    });
                auto planIter = upper_bound(MAAPlans.begin(), MAAPlans.end(), curSecond,
                    [](const int& a, const MAADailyTaskPlan& b) {
                        return a < b.taskSeconds;
                    });
                bool nextDay = false;
                if (planIter == MAAPlans.end()) {
                    planIter = MAAPlans.begin();
                    nextDay = true;
                }
                std::tm nextDailyTaskTime = tmNow;
                auto nextDailyTaskTimeTm = stringToTm(planIter->dailyTaskTime, "%H:%M:%S");
                nextDailyTaskTime.tm_hour = nextDailyTaskTimeTm.tm_hour;
                nextDailyTaskTime.tm_min = nextDailyTaskTimeTm.tm_min;
                nextDailyTaskTime.tm_sec = nextDailyTaskTimeTm.tm_sec;
                if (nextDay) {
                    auto newTm = DateAdd(nextDailyTaskTime, 24 * 60 * 60);
                    nextDailyTaskTime = newTm;
                }
                std::string strTaskEndTime = tmToString(tmNow, "%Y-%m-%d %H:%M:%S");
                std::unordered_map<std::string, std::string> updateColMap;
                updateColMap["dailyTaskEndTime"] = strTaskEndTime;
                updateColMap["nextDailyTaskTime"] = tmToString(nextDailyTaskTime, "%Y-%m-%d %H:%M:%S");
                bool updateRes = updateMAAUser(conn, userID, deviceID, updateColMap);
            }
        } else {
            auto MAAActions = queryMAAAction(conn, returnTaskID);
            if (!MAAActions.empty()) {
                updateMAAAction(conn, returnTaskID, "1");
                auto quickTaskID = MAAActions[0].taskID;
                auto unFinishActions = queryMAAAction(conn, quickTaskID, "0");
                if (unFinishActions.empty()) {
                    std::unordered_map<std::string, std::string> updateColMap;
                    updateColMap["taskIsFinish"] = "1";
                    updateMAAQuickTask(conn, quickTaskID, updateColMap);
                }
            }
        }
    }
    co_await connPool->ReleaseConnection(conn);
    co_await co_await HTTPServerUtils::make_ok_response(io,
        "<h1>reportStatus!</h1>");
    co_return {};
}