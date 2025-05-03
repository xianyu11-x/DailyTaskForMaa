#include "co_async/awaiter/task.hpp"
#include "co_async/utils/expected.hpp"
#include <co_async/co_async.hpp>
#include <co_async/std.hpp>
#pragma once
using namespace co_async;

Task<Expected<>> updateStrategy(HTTPServer::IO& io);

std::string parseSimpleStrategy(std::string& strategyStr);

bool checkStrategy(std::string& strategyStr);