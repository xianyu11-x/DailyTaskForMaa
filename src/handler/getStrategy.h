#include "co_async/awaiter/task.hpp"
#include "co_async/utils/expected.hpp"
#include "rapidjson/document.h"
#include <co_async/co_async.hpp>
#include <co_async/std.hpp>
#include <mysql/mysql.h>
#pragma once
using namespace co_async;

Task<Expected<>> getStrategy(HTTPServer::IO& io);