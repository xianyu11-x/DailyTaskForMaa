#include "co_async/awaiter/task.hpp"
#include "co_async/utils/expected.hpp"
#include "rapidjson/document.h"
#include <co_async/co_async.hpp>
#include <co_async/std.hpp>
#include <mysql/mysql.h>
#pragma once
using namespace co_async;

rapidjson::Document getDefaultStrategy();

std::string getDefaultLevel(const rapidjson::Value::ConstArray& levelList);

rapidjson::Document getDailyTask(const std::string& coreTaskId, const rapidjson::Value::Array& taskStrategy);

rapidjson::Document getQuickTask(const rapidjson::Value::Array& taskStrategy);

int userInit(std::string userID, std::string deviceID, MYSQL* conn);

Task<Expected<>> getTask(HTTPServer::IO& io);