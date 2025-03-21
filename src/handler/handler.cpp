#include "handler.h"
#include "../conf/conf.h"
#include "../levelManager/levelManager.h"
#include "../mysqlConnectPool/sqlConnectPool.h"
#include "../sqlMap/sqlMap.h"
#include "../utils/jsonUtil.hpp"
#include "../utils/timeUtil.hpp"
#include "../utils/uuid.hpp"
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

rapidjson::Document getDefaultStrategy()
{
    rapidjson::Document defaultStrategy;
    defaultStrategy.SetObject();
    auto& allocator = defaultStrategy.GetAllocator();
    auto confManager = confManager::GetInstance();
    auto dailyTaskParams = confManager->getDefaultLevelList();
    // vector<std::string> dailyTaskParams = {"AP-5", "CE-6",   "PR-C-2",
    //                                        "AP-5", "PR-A-2", "CE-6"};
    for (int i = 1; i <= 7; i++) {
        rapidjson::Value strategyArray(rapidjson::kArrayType);
        rapidjson::Value task1(rapidjson::kObjectType);
        task1.AddMember("taskType", "Settings-Stage1", allocator);
        rapidjson::Value paramsArray(rapidjson::kArrayType);
        for (const auto& level : dailyTaskParams[i - 1]) {
            paramsArray.PushBack(rapidjson::Value(level.c_str(), allocator),
                allocator);
        }
        task1.AddMember("params", paramsArray, allocator);
        strategyArray.PushBack(task1, allocator);
        rapidjson::Value task2(rapidjson::kObjectType);
        task2.AddMember("taskType", "LinkStart", allocator);
        strategyArray.PushBack(task2, allocator);
        rapidjson::Value key(std::to_string(i).c_str(), allocator);
        defaultStrategy.AddMember(key, strategyArray, allocator);
    }
    return defaultStrategy;
}

std::string getDefaultLevel(const rapidjson::Value::ConstArray& levelList)
{
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm tmNow = *std::localtime(&nowTime);
    int dayOfWeek = tmNow.tm_wday;
    if (tmNow.tm_hour < 4) {
        dayOfWeek = (dayOfWeek + 6) % 7;
    }
    auto levelManager = levelManager::GetInstance();
    std::string levelName = "1-7";
    for (const auto& level : levelList) {
        std::string curLevelName = level.GetString();
        if (curLevelName == "sideStory") {
            auto [sideStoryStartTime, sideStoryEndTime] = levelManager->getSideStoryTime();
            if (sideStoryStartTime == "" || sideStoryEndTime == "") {
                continue;
            }
            auto sideStoryStartTimeTm = stringToTm(sideStoryStartTime, "%Y/%m/%d %H:%M:%S");
            auto sideStoryEndTimeTm = stringToTm(sideStoryEndTime, "%Y/%m/%d %H:%M:%S");
            if (isTimeAfter(tmNow, sideStoryStartTimeTm) && isTimeAfter(sideStoryEndTimeTm, tmNow)) {
                return levelManager->getDefaultSideStoryLevel();
            }
        } else if (levelManager->checkLevelStatus(curLevelName, dayOfWeek)) {
            return curLevelName;
        }
    }
    return levelName;
}

rapidjson::Document getDailyTask(const std::string& coreTaskId,
    const rapidjson::Value::Array& taskStrategy)
{
    rapidjson::Document dailyTask;
    dailyTask.SetObject();
    auto& allocator = dailyTask.GetAllocator();
    rapidjson::Value taskArray(rapidjson::kArrayType);
    for (const auto& task : taskStrategy) {
        rapidjson::Value taskObj(rapidjson::kObjectType);
        std::string taskType = task["taskType"].GetString();
        if (taskType == "LinkStart") {
            rapidjson::Value coreTaskIdValue(coreTaskId.c_str(), allocator);
            taskObj.AddMember("id", coreTaskIdValue, allocator);
        } else {
            rapidjson::Value taskIDValue(generateUUID().c_str(), allocator);
            taskObj.AddMember("id", taskIDValue, allocator);
        }
        rapidjson::Value taskTypeValue(taskType.c_str(), allocator);
        taskObj.AddMember("type", taskTypeValue, allocator);
        if (task.HasMember("params")) {
            auto params = task["params"].GetArray();
            if (taskType == "Settings-Stage1") {
                auto curlevelName = getDefaultLevel(params);
                rapidjson::Value paramsValue(curlevelName.c_str(), allocator);
                taskObj.AddMember("params", paramsValue, allocator);
            }
        }
        taskArray.PushBack(taskObj, allocator);
    }
    dailyTask.AddMember("tasks", taskArray, allocator);
    return dailyTask;
}

rapidjson::Document getQuickTask(const rapidjson::Value::Array& taskStrategy)
{
    rapidjson::Document quickTask;
    quickTask.SetObject();
    auto& allocator = quickTask.GetAllocator();
    rapidjson::Value taskArray(rapidjson::kArrayType);
    for (const auto& task : taskStrategy) {
        rapidjson::Value taskObj(rapidjson::kObjectType);
        std::string taskType = task["type"].GetString();
        std::string taskID = task["id"].GetString();
        rapidjson::Value taskIDValue(taskID.c_str(), allocator);
        taskObj.AddMember("id", taskIDValue, allocator);
        rapidjson::Value taskTypeValue(taskType.c_str(), allocator);
        taskObj.AddMember("type", taskTypeValue, allocator);
        taskArray.PushBack(taskObj, allocator);
    }
    quickTask.AddMember("tasks", taskArray, allocator);
    debug(), jsonToString(quickTask);
    return quickTask;
}

int userInit(std::string userID, std::string deviceID, MYSQL* conn)
{
    auto defaultStrategy = getDefaultStrategy();
    std::string defaultStrategyStr = jsonToString(defaultStrategy);

    auto confManager = confManager::GetInstance();
    auto& dailyTaskTimeList = confManager->getDefaultDailyTaskTimeList();
    vector<MAADailyTaskPlan> initMAAPlanList;
    for (const auto& dailyTaskTime : dailyTaskTimeList) {
        initMAAPlanList.push_back(
            MAADailyTaskPlan { .planID = generateUUID(),
                .userID = userID,
                .deviceID = deviceID,
                .dailyTaskStrategy = defaultStrategyStr,
                .dailyTaskTime = dailyTaskTime });
    }
    int resPlan = insertMAADailyTaskPlan(conn, initMAAPlanList);
    if (resPlan == -1) {
        return resPlan;
    }
    // 生成默认的每日任务时间
    std::tm dailyTaskTime = getNowTm();
    dailyTaskTime.tm_year--;
    std::string strNextTaskTime = tmToString(dailyTaskTime, "%Y-%m-%d %H:%M:%S");
    dailyTaskTime.tm_year--;
    std::string strTaskTime = tmToString(dailyTaskTime, "%Y-%m-%d %H:%M:%S");
    vector<MAAUser> initMAAUserList;
    initMAAUserList.push_back(MAAUser { .userID = userID,
        .deviceID = deviceID,
        .nextDailyTaskTime = strNextTaskTime,
        .dailyTaskStartTime = strTaskTime,
        .dailyTaskEndTime = strTaskTime,
        .dailyTaskID = "" });
    int res = insertMAAUser(conn, initMAAUserList);
    return res;
}

Task<Expected<>> getTask(HTTPServer::IO& io)
{

    auto connPool = connectionPool::GetInstance();
    auto conn = co_await connPool->GetConnection();

    auto request = co_await io.request_body();

    rapidjson::Document requestDOM;
    rapidjson::Document responseDOM;
    requestDOM.Parse(request.value().c_str());

    // TODO:检查request是否合法

    std::string userID = requestDOM["user"].GetString();
    std::string deviceID = requestDOM["device"].GetString();

    auto curUserInfos = queryMAAUserAllInfo(conn, userID, deviceID);

    bool hasTask = false;
    if (curUserInfos.empty()) {
        // 向数据库插入数据
        int rlt = userInit(userID, deviceID, conn);
        if (rlt == -1) {
            co_await co_await stdio().putline("init userInfo error"s);
        }
    } else {
        auto curUserInfo = curUserInfos[0];
        std::tm tmNow = getNowTm();
        // 检查当前时间是否超过了dailyTaskTime并且taskStartTime小于dailyTaskTime(下发任务)
        // 检查taskStartTime是否超过了dailyTaskTime并且当前时间超过taskStartTime2个小时(重发任务)
        // 其他情况不更新任务状态

        auto tmDailyTaskTime = stringToTm(curUserInfo.nextDailyTaskTime, "%Y-%m-%d %H:%M:%S");
        auto tmTaskStartTime = stringToTm(curUserInfo.dailyTaskStartTime, "%Y-%m-%d %H:%M:%S");
        auto temptm = std::mktime(&tmTaskStartTime) + 2 * 60 * 60;

        auto tmExpireTaskStartTime = *std::localtime(&temptm);
        std::string strNewStartTaskTime = tmToString(tmNow, "%Y-%m-%d %H:%M:%S");

        if ((isTimeAfter(tmNow, tmDailyTaskTime) && isTimeAfter(tmDailyTaskTime, tmTaskStartTime)) || (isTimeAfter(tmTaskStartTime, tmDailyTaskTime) && isTimeAfter(tmNow, tmExpireTaskStartTime))) {
            // 更新任务状态&&重发任务
            debug(), "send task to"s, curUserInfo.userID;

            std::string curTaskID = generateUUID();
            std::unordered_map<std::string, std::string> updateColMap;
            updateColMap["dailyTaskStartTime"] = strNewStartTaskTime;
            updateColMap["dailyTaskID"] = curTaskID;

            // 构造任务响应
            int dayOfWeek = tmNow.tm_wday;
            if (tmNow.tm_hour < 4) {
                dayOfWeek = (dayOfWeek + 6) % 7;
            }
            if (dayOfWeek == 0) {
                dayOfWeek = 7;
            }
            // TODO:根据MAADailyTaskPlan获取任务策略
            auto MAAPlans = queryMAAUserStrategy(conn, curUserInfo.userID, curUserInfo.deviceID);
            for (auto& MAAPlan : MAAPlans) {
                auto dailyTaskTimeTm = stringToTm(MAAPlan.dailyTaskTime, "%H:%M:%S");
                MAAPlan.taskSeconds = (dailyTaskTimeTm.tm_hour * 3600 + dailyTaskTimeTm.tm_min * 60 + dailyTaskTimeTm.tm_sec);
            }
            std::sort(MAAPlans.begin(), MAAPlans.end(),
                [](const MAADailyTaskPlan& a, const MAADailyTaskPlan& b) {
                    return a.taskSeconds < b.taskSeconds;
                });
            auto planIter = upper_bound(MAAPlans.begin(), MAAPlans.end(),
                tmNow.tm_hour * 3600 + tmNow.tm_min * 60 + tmNow.tm_sec,
                [](const int& a, const MAADailyTaskPlan& b) {
                    return a < b.taskSeconds;
                });
            if (planIter == MAAPlans.begin()) {
                planIter = MAAPlans.end();
            }
            planIter--;
            auto taskStrategy = stringToJson(planIter->dailyTaskStrategy);
            responseDOM = getDailyTask(
                curTaskID,
                taskStrategy[std::to_string(dayOfWeek).c_str()].GetArray());

            bool updateRes = updateMAAUser(conn, curUserInfo.userID,
                curUserInfo.deviceID, updateColMap);
            if (!updateRes) {
                debug(), "update task error"s;
            }
            hasTask = true;
        } else {
            // TODO:检查有没有quickTask需要下发
            debug(), "no need to update task"s, curUserInfo.userID;
            string taskStr;
            auto quickTasks = queryMAAQuickTask(conn, curUserInfo.userID,
                curUserInfo.deviceID, "0");
            if (!quickTasks.empty()) {
                bool hasQuickTask = false;
                for (const auto& quickTask : quickTasks) {
                    auto quickTaskStartTime = stringToTm(quickTask.taskStartTime, "%Y-%m-%d %H:%M:%S");
                    auto quickTaskCommitTime = stringToTm(quickTask.taskCommitTime, "%Y-%m-%d %H:%M:%S");
                    auto quickTaskExpireTime = std::mktime(&quickTaskStartTime) + 2 * 60 * 60;
                    auto tmExpireQuickTaskStartTime = *std::localtime(&quickTaskExpireTime);
                    if (isTimeAfter(quickTaskCommitTime, quickTaskStartTime) || isTimeAfter(tmNow, tmExpireQuickTaskStartTime)) {
                        taskStr = quickTask.taskActions;
                        std::unordered_map<std::string, std::string> updateColMap;
                        updateColMap["taskStartTime"] = strNewStartTaskTime;
                        updateMAAQuickTask(conn, quickTask.taskID, updateColMap);
                        hasQuickTask = true;
                        break;
                    }
                }
                if (hasQuickTask) {
                    auto quickTaskList = stringToJson(taskStr);
                    auto quickUuid = generateUUID();
                    responseDOM = getQuickTask(quickTaskList.GetArray());
                    hasTask = true;
                }
            }
        }
    }
    if (!hasTask) {
        responseDOM.SetObject();
        auto& allocator = responseDOM.GetAllocator();
        rapidjson::Value taskArray(rapidjson::kArrayType);
        rapidjson::Value taskHeartbeat(rapidjson::kObjectType);
        std::string uuid = generateUUID();
        rapidjson::Value id(uuid.c_str(), allocator);
        taskHeartbeat.AddMember("id", id, allocator);
        taskHeartbeat.AddMember("type", "HeartBeat", allocator);
        taskArray.PushBack(taskHeartbeat, allocator);
        responseDOM.AddMember("tasks", taskArray, allocator);
    }
    auto responseStr = jsonToString(responseDOM);
    debug(), responseStr;
    co_await connPool->ReleaseConnection(conn);
    co_await co_await HTTPServerUtils::make_ok_response(io, responseStr);
    co_return {};
}

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
                updateColMap["taskEndTime"] = strTaskEndTime;
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

Task<Expected<>> updateLevel(HTTPServer::IO& io)
{
    auto request = co_await io.request_body();
    rapidjson::Document requestDOM;
    requestDOM.Parse(request.value().c_str());
    auto levelManager = levelManager::GetInstance();
    auto sideStoryLevelList = requestDOM["Official"]["sideStoryStage"].GetArray();
    std::vector<std::string> levelList;
    std::string startTime;
    std::string endTime;
    bool hasAvailableTime = false;
    std::tm tmNow = getNowTm();
    for (const auto& level : sideStoryLevelList) {
        auto tmpStartTime = level["Activity"]["UtcStartTime"].GetString();
        auto tmpEndTime = level["Activity"]["UtcExpireTime"].GetString();
        auto levelName = level["Value"].GetString();
        if (isTimeAfter(tmNow, stringToTm(tmpStartTime, "%Y/%m/%d %H:%M:%S")) && isTimeAfter(stringToTm(tmpEndTime, "%Y/%m/%d %H:%M:%S"), tmNow)) {
            levelList.push_back(levelName);
            if (!hasAvailableTime) {
                startTime = tmpStartTime;
                endTime = tmpEndTime;
                hasAvailableTime = true;
            }
        }
    }
    if (!levelList.empty())
        levelManager->setSideStoryLevel(levelList, startTime, endTime);
    co_await co_await HTTPServerUtils::make_ok_response(io,
        "updateSideStoryLevel");
    co_return {};
}

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