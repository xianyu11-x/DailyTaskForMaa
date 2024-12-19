#include "handler.h"
#include "../mysqlConnectPool/sqlConnectPool.h"
#include "../sqlMap/sqlMap.h"
#include "co_async/utils/expected.hpp"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <mysql/mysql.h>
#include <sstream>

rapidjson::Document getDefaultStrategy() {
  rapidjson::Document defaultStrategy;
  defaultStrategy.SetObject();
  auto &allocator = defaultStrategy.GetAllocator();
  rapidjson::Value strategyArray(rapidjson::kArrayType);
  rapidjson::Value task1(rapidjson::kObjectType);
  task1.AddMember("taskType", "Settings-Stage1", allocator);
  task1.AddMember("params", "Default", allocator);
  strategyArray.PushBack(task1, allocator);
  rapidjson::Value task2(rapidjson::kObjectType);
  task2.AddMember("taskType", "LinkStart", allocator);
  strategyArray.PushBack(task2, allocator);
  for (int i = 1; i < 7; i++) {
    rapidjson::Value key(std::to_string(i).c_str(), allocator);
    defaultStrategy.AddMember(key, strategyArray, allocator);
  }
  rapidjson::Value sundayStrategyArray(rapidjson::kArrayType);
  rapidjson::Value sundayTask1(rapidjson::kObjectType);
  sundayTask1.AddMember("taskType", "Settings-Stage1", allocator);
  sundayTask1.AddMember("params", "剿灭作战", allocator);
  sundayStrategyArray.PushBack(sundayTask1, allocator);
  rapidjson::Value sundayTask2(rapidjson::kObjectType);
  sundayTask2.AddMember("taskType", "LinkStart", allocator);
  sundayStrategyArray.PushBack(sundayTask2, allocator);
  defaultStrategy.AddMember("7", sundayStrategyArray, allocator);
  return defaultStrategy;
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
  requestDOM.Parse(request.value().c_str());

  std::string userID = requestDOM["user"].GetString();
  std::string deviceID = requestDOM["device"].GetString();

  string queryStr = "select * from MAAUser where UserID = " + userID +
                    " and DeviceID = " + deviceID;
  mysql_query(conn, queryStr.c_str());

  auto res = mysql_store_result(conn);
  if (res == nullptr) {
    // Log error
  }

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
    //检查当前时间是否超过了dailyTaskTime并且taskStartTime小于dailyTaskTime(下发任务)    
    //检查当前时间是否超过了dailyTaskTime并且当前时间超过taskStartTime3个小时(重发任务)
    //其他情况不更新任务状态

    
  }

  rapidjson::Document responseDOM;

  co_await connPool->ReleaseConnection(conn);
  co_await co_await HTTPServerUtils::make_ok_response(io, "<h1>getTask!</h1>");
  co_return {};
}

Task<Expected<>> reportStatus(HTTPServer::IO &io) {
  co_await co_await HTTPServerUtils::make_ok_response(io,
                                                      "<h1>reportStatus!</h1>");
  co_return {};
}
