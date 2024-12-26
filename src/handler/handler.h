#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;

Task<Expected<>> getTask(HTTPServer::IO &io);

Task<Expected<>> reportStatus(HTTPServer::IO &io);

Task<Expected<>> updateLevel(HTTPServer::IO &io);