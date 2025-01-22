#include "co_async/awaiter/task.hpp"
#include "co_async/utils/expected.hpp"
#include <co_async/co_async.hpp>
#include <co_async/std.hpp>
#pragma once
using namespace co_async;

Task<Expected<>> getTask(HTTPServer::IO &io);

Task<Expected<>> reportStatus(HTTPServer::IO &io);

Task<Expected<>> updateLevel(HTTPServer::IO &io);

Task<Expected<>> updateStrategy(HTTPServer::IO &io);

Task<Expected<>> getStrategy(HTTPServer::IO &io);

Task<Expected<>> quickTask(HTTPServer::IO &io);