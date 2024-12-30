#include "handler.h"
#include "../levelManager/levelManager.h"
#include "../mysqlConnectPool/sqlConnectPool.h"
#include "../sqlMap/sqlMap.h"
#include "../utils/jsonUtil.hpp"
#include "../utils/timeUtil.hpp"
#include "../utils/uuid.hpp"
#include "co_async/awaiter/task.hpp"
#include "co_async/generic/allocator.hpp"
#include "co_async/utils/expected.hpp"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <chrono>
#include <ctime>
#include <functional>
#include <iomanip>
#include <mysql/mysql.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

rapidjson::Document getDefaultStrategy() {
  rapidjson::Document defaultStrategy;
  defaultStrategy.SetObject();
  auto &allocator = defaultStrategy.GetAllocator();
  // TODO：默认策略保存在配置文件中
  vector<std::string> dailyTaskParams = {"AP-5", "CE-6",   "PR-C-2",
                                         "AP-5", "PR-A-2", "CE-6"};
  for (int i = 1; i < 7; i++) {
    rapidjson::Value strategyArray(rapidjson::kArrayType);
    rapidjson::Value task1(rapidjson::kObjectType);
    task1.AddMember("taskType", "Settings-Stage1", allocator);
    rapidjson::Value paramsArray(rapidjson::kArrayType);
    paramsArray.PushBack(rapidjson::Value("sideStory", allocator), allocator);
    paramsArray.PushBack(
        rapidjson::Value(dailyTaskParams[i - 1].c_str(), allocator), allocator);
    task1.AddMember("params", paramsArray, allocator);
    strategyArray.PushBack(task1, allocator);
    rapidjson::Value task2(rapidjson::kObjectType);
    task2.AddMember("taskType", "LinkStart", allocator);
    strategyArray.PushBack(task2, allocator);
    rapidjson::Value key(std::to_string(i).c_str(), allocator);
    defaultStrategy.AddMember(key, strategyArray, allocator);
  }
  rapidjson::Value sundayStrategyArray(rapidjson::kArrayType);
  rapidjson::Value sundayTask1(rapidjson::kObjectType);
  sundayTask1.AddMember("taskType", "Settings-Stage1", allocator);
  rapidjson::Value paramsArray(rapidjson::kArrayType);
  paramsArray.PushBack(rapidjson::Value("剿灭模式", allocator), allocator);
  sundayTask1.AddMember("params", paramsArray, allocator);
  sundayStrategyArray.PushBack(sundayTask1, allocator);
  rapidjson::Value sundayTask2(rapidjson::kObjectType);
  sundayTask2.AddMember("taskType", "LinkStart", allocator);
  sundayStrategyArray.PushBack(sundayTask2, allocator);
  defaultStrategy.AddMember("7", sundayStrategyArray, allocator);
  return defaultStrategy;
}

std::string getDefaultLevel(const rapidjson::Value::ConstArray &levelList) {
  auto now = std::chrono::system_clock::now();
  std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
  std::tm tmNow = *std::localtime(&nowTime);
  int dayOfWeek = tmNow.tm_wday;
  if (tmNow.tm_hour < 4) {
    dayOfWeek = (dayOfWeek + 6) % 7;
  }
  auto levelManager = levelManager::GetInstance();
  std::string levelName = "1-7";
  for (const auto &level : levelList) {
    std::string curLevelName = level.GetString();
    if (curLevelName == "sideStory") {
      auto [sideStoryStartTime, sideStoryEndTime] =
          levelManager->getSideStoryTime();
      if (sideStoryStartTime == "" || sideStoryEndTime == "") {
        continue;
      }
      auto sideStoryStartTimeTm =
          stringToTm(sideStoryStartTime, "%Y/%m/%d %H:%M:%S");
      auto sideStoryEndTimeTm =
          stringToTm(sideStoryEndTime, "%Y/%m/%d %H:%M:%S");
      if (isTimeAfter(tmNow, sideStoryStartTimeTm) &&
          isTimeAfter(sideStoryEndTimeTm, tmNow)) {
        return levelManager->getDefaultSideStoryLevel();
      }
    } else if (levelManager->checkLevelStatus(curLevelName, dayOfWeek)) {
      return curLevelName;
    }
  }
  return levelName;
}

rapidjson::Document getDailyTask(const std::string &coreTaskId,
                                 const rapidjson::Value::Array &taskStrategy) {
  rapidjson::Document dailyTask;
  dailyTask.SetObject();
  auto &allocator = dailyTask.GetAllocator();
  rapidjson::Value taskArray(rapidjson::kArrayType);
  for (const auto &task : taskStrategy) {
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

int userInit(std::string userID, std::string deviceID, MYSQL *conn) {
  // TODO:从json文件中读取用户信息
  auto defaultStrategy = getDefaultStrategy();
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  defaultStrategy.Accept(writer);
  std::string defaultStrategyStr = buffer.GetString();

  // 生成默认的每日任务时间
  auto now = std::chrono::system_clock::now();
  std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
  std::tm dailyTaskTime = *std::localtime(&nowTime);
  dailyTaskTime.tm_hour = 1;
  dailyTaskTime.tm_min = 0;
  dailyTaskTime.tm_sec = 0;
  std::ostringstream oss;
  oss << std::put_time(&dailyTaskTime, "%Y-%m-%d %H:%M:%S");
  std::string strTaskTime = oss.str();

  // 设置当前任务默认状态
  dailyTaskTime.tm_year--;
  oss.str("");
  oss << std::put_time(&dailyTaskTime, "%Y-%m-%d %H:%M:%S");
  std::string curTaskDefaultTime = oss.str();

  int res = insertMAAUserInit(conn, userID, deviceID, defaultStrategyStr,
                              strTaskTime, curTaskDefaultTime);
  return res;
}

Task<Expected<>> getTask(HTTPServer::IO &io) {

  auto connPool = connectionPool::GetInstance();
  auto conn = co_await connPool->GetConnection();

  auto request = co_await io.request_body();

  rapidjson::Document requestDOM;
  rapidjson::Document responseDOM;
  requestDOM.Parse(request.value().c_str());

  // TODO:检查request是否合法

  std::string userID = requestDOM["user"].GetString();
  std::string deviceID = requestDOM["device"].GetString();

  string queryStr = "select * from MAAUser where userID = \"" + userID +
                    "\" and deviceID = \"" + deviceID + "\"";
  mysql_query(conn, queryStr.c_str());

  auto res = mysql_store_result(conn);
  if (res == nullptr) {
    // Log error
  }
  bool hasTask = false;
  if (mysql_num_rows(res) == 0) {
    // 向数据库插入数据
    int rlt = userInit(userID, deviceID, conn);
    if (rlt == -1) {
      co_await co_await stdio().putline("init userInfo error"s);
    }
  } else if (mysql_num_rows(res) > 1) {
    co_await co_await stdio().putline("userInfo nums error"s);
  } else {
    int num_fields = mysql_num_fields(res);
    MYSQL_ROW row = mysql_fetch_row(res);
    MAAUser user{.userID = row[0] ? row[0] : "",
                 .deviceID = row[1] ? row[1] : "",
                 .dailyTaskTime = row[2] ? row[2] : "",
                 .taskStartTime = row[3] ? row[3] : "",
                 .taskEndTime = row[4] ? row[4] : "",
                 .taskStrategy = row[5] ? row[5] : "",
                 .dailyTaskID = row[6] ? row[6] : ""};
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm tmNow = *std::localtime(&nowTime);
    // 检查当前时间是否超过了dailyTaskTime并且taskStartTime小于dailyTaskTime(下发任务)
    // 检查taskStartTime是否超过了dailyTaskTime并且当前时间超过taskStartTime2个小时(重发任务)
    // 其他情况不更新任务状态

    auto tmDailyTaskTime = stringToTm(user.dailyTaskTime, "%Y-%m-%d %H:%M:%S");
    auto tmTaskStartTime = stringToTm(user.taskStartTime, "%Y-%m-%d %H:%M:%S");
    auto temptm = std::mktime(&tmTaskStartTime) + 2 * 60 * 60;

    auto tmExpectedTaskStartTime = *std::localtime(&temptm);
    std::ostringstream oss;
    oss << std::put_time(&tmNow, "%Y-%m-%d %H:%M:%S");
    std::string strNewStartTaskTime = oss.str();

    if ((isTimeAfter(tmNow, tmDailyTaskTime) &&
         isTimeAfter(tmDailyTaskTime, tmTaskStartTime)) ||
        (isTimeAfter(tmTaskStartTime, tmDailyTaskTime) &&
         isTimeAfter(tmNow, tmExpectedTaskStartTime))) {
// 更新任务状态&&重发任务
#if CO_ASYNC_DEBUG
      co_await co_await stdio().putline("send task to"s + user.userID);
#endif
      std::string curTaskID = generateUUID();
      std::unordered_map<std::string, std::string> updateColMap;
      updateColMap["taskStartTime"] = strNewStartTaskTime;
      updateColMap["dailyTaskID"] = curTaskID;

      // 构造任务响应
      int dayOfWeek = tmNow.tm_wday;
      if (tmNow.tm_hour < 4) {
        dayOfWeek = (dayOfWeek + 6) % 7;
      }
      if (dayOfWeek == 0) {
        dayOfWeek = 7;
      }
      auto taskStrategy = stringToJson(user.taskStrategy);

      responseDOM = getDailyTask(
          curTaskID,
          taskStrategy[std::to_string(dayOfWeek).c_str()].GetArray());

      bool updateRes =
          updateMAAUser(conn, user.userID, user.deviceID, updateColMap);
      if (!updateRes) {
#if CO_ASYNC_DEBUG
        co_await co_await stdio().putline("update task error"s);
#endif
      }
      hasTask = true;
    } else {
#if CO_ASYNC_DEBUG
      co_await co_await stdio().putline("no need to update task"s +
                                        user.userID);
#endif
    }
  }
  if (!hasTask) {
    responseDOM.SetObject();
    auto &allocator = responseDOM.GetAllocator();
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
  co_await connPool->ReleaseConnection(conn);
  co_await co_await HTTPServerUtils::make_ok_response(io, responseStr);
  co_return {};
}

Task<Expected<>> reportStatus(HTTPServer::IO &io) {
  auto connPool = connectionPool::GetInstance();
  auto conn = co_await connPool->GetConnection();

  auto request = co_await io.request_body();

  rapidjson::Document requestDOM;
  rapidjson::Document responseDOM;
  requestDOM.Parse(request.value().c_str());
  std::string userID = requestDOM["user"].GetString();
  std::string deviceID = requestDOM["device"].GetString();
  std::string returnTaskID = requestDOM["task"].GetString();
  auto storeUserInfo = queryMAAUserInfo(conn, userID, deviceID);
  if (storeUserInfo.userID == "" || storeUserInfo.deviceID == "") {
    co_await co_await stdio().putline("userInfo not exist"s);
  } else {
    auto storeTaskInfo = queryMAAUserTaskStatus(conn, storeUserInfo.userID,
                                                storeUserInfo.deviceID);
    if (storeTaskInfo.dailyTaskID == returnTaskID) {
      auto now = std::chrono::system_clock::now();
      std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
      std::tm tmNow = *std::localtime(&nowTime);
      std::string strTaskEndTime = tmToString(tmNow, "%Y-%m-%d %H:%M:%S");

      auto curDailyTaskTime =
          stringToTm(storeTaskInfo.dailyTaskTime, "%Y-%m-%d %H:%M:%S");
      auto nextDailyTaskTime = DateAdd(curDailyTaskTime, 60 * 60 * 24);
      std::unordered_map<std::string, std::string> updateColMap;
      updateColMap["taskEndTime"] = strTaskEndTime;
      updateColMap["dailyTaskTime"] =
          tmToString(nextDailyTaskTime, "%Y-%m-%d %H:%M:%S");
      bool updateRes = updateMAAUser(conn, storeUserInfo.userID,
                                     storeUserInfo.deviceID, updateColMap);
      if (!updateRes) {
#if CO_ASYNC_DEBUG
        co_await co_await stdio().putline("update task error"s);
#endif
      }
    }
  }

  co_await connPool->ReleaseConnection(conn);
  co_await co_await HTTPServerUtils::make_ok_response(io,
                                                      "<h1>reportStatus!</h1>");
  co_return {};
}

Task<Expected<>> updateLevel(HTTPServer::IO &io) {
  auto request = co_await io.request_body();
  rapidjson::Document requestDOM;
  requestDOM.Parse(request.value().c_str());
  auto levelManager = levelManager::GetInstance();
  auto sideStoryLevelList = requestDOM["Official"]["sideStoryStage"].GetArray();
  if (!sideStoryLevelList.Empty()) {
    auto startTime =
        sideStoryLevelList[0]["Activity"]["UtcStartTime"].GetString();
    auto endTime =
        sideStoryLevelList[0]["Activity"]["UtcExpireTime"].GetString();
    std::cout << startTime << std::endl;
    std::cout << endTime << std::endl;
    levelManager->setSideStoryLevel(sideStoryLevelList, startTime, endTime);
  }
  co_await co_await HTTPServerUtils::make_ok_response(io,
                                                      "updateSideStoryLevel");
  co_return {};
}

Task<Expected<>> updateStrategy(HTTPServer::IO &io) {
  auto request = co_await io.request_body();
  rapidjson::Document requestDOM;
  requestDOM.Parse(request.value().c_str());
  // TODO:合法性校验
  auto connPool = connectionPool::GetInstance();
  auto conn = co_await connPool->GetConnection();
  auto userID = requestDOM["user"].GetString();
  auto deviceID = requestDOM["device"].GetString();
  auto taskStrategy = requestDOM["strategy"].GetString();
  std::unordered_map<std::string, std::string> updateColMap;
  updateColMap["taskStrategy"] = taskStrategy;
  bool updateRes = updateMAAUser(conn, userID, deviceID, updateColMap);
  if (!updateRes) {
    co_await co_await stdio().putline("update task error"s);
  }
  co_await connPool->ReleaseConnection(conn);
  co_await co_await HTTPServerUtils::make_ok_response(io, "updateStrategy");
  co_return {};
}

Task<Expected<>> getStrategy(HTTPServer::IO &io) {
  auto connPool = connectionPool::GetInstance();
  auto conn = co_await connPool->GetConnection();
  auto request = co_await io.request_body();
  rapidjson::Document requestDOM;
  requestDOM.Parse(request.value().c_str());
  auto userID = requestDOM["user"].GetString();
  auto deviceID = requestDOM["device"].GetString();
  auto storeUserInfo = queryMAAUserStrategy(conn, userID, deviceID);
  if (storeUserInfo.userID == "" || storeUserInfo.deviceID == "") {
#ifdef CO_ASYNC_DEBUG
    co_await co_await stdio().putline("userInfo not exist"s);
#endif
    co_await co_await HTTPServerUtils::make_ok_response(io,
                                                        "userInfo not exist");
  } else {
    auto taskStrategy = storeUserInfo.taskStrategy;
#ifdef CO_ASYNC_DEBUG
    co_await co_await stdio().putline(taskStrategy);
#endif
    co_await co_await HTTPServerUtils::make_ok_response(io, taskStrategy);
  }
  co_await connPool->ReleaseConnection(conn);

  co_return {};
}