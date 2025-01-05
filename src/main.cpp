#include "co_async/awaiter/task.hpp"
#include "levelManager/levelManager.h"
#include "mysqlConnectPool/sqlConnectPool.h"
#include "router/router.h"
#include <co_async/co_async.hpp>
#include <co_async/std.hpp>
#include "conf/conf.h"

using namespace co_async;
using namespace std::literals;

static Task<Expected<>> amain(std::string serveAt,std::string confPath) {
  co_await co_await stdio().putline("listening at: "s + serveAt);
  auto listener = co_await co_await listener_bind(
      co_await AddressResolver().host(serveAt).resolve_one());

  auto confPathStr = make_path(".",confPath);
  auto confStr = co_await co_await file_read(confPathStr);
  auto confManager = confManager::GetInstance();
  confManager->init(confStr);

  auto permanentLevelpath = make_path(".",confManager->getPermantLevelPath());
  auto permanentLevelStr = co_await co_await file_read(permanentLevelpath);
  auto levelManager = levelManager::GetInstance();
  levelManager->setPermanentLevel(permanentLevelStr);

  //TODO:使用配置文件统一处理配置信息
  co_await co_await stdio().putline("connecting db"s );
  auto dbInfo = confManager->getDbInfo();
  auto conn = connectionPool::GetInstance();
  conn->init(dbInfo.url, dbInfo.user, dbInfo.password, dbInfo.databaseName, dbInfo.port, dbInfo.maxConn);
  HTTPServer server;
  co_await co_await stdio().putline("setting route"s );
  setAllRoute(server);

  struct Worker {
    std::deque<Task<Expected<>>> q;
    std::condition_variable cv;
    std::mutex mtx;
    std::jthread th;

    void spawn(Task<Expected<>> task) {
      std::lock_guard lck(mtx);
      q.push_back(std::move(task));
      cv.notify_all();
    }

    void start(std::size_t i) {
      th = std::jthread([this, i](std::stop_token stop) {
        IOContext ctx;
        PlatformIOContext::schedSetThreadAffinity(i);
        while (!stop.stop_requested()) [[likely]] {
          while (ctx.runOnce()) {
            std::lock_guard lck(mtx);
            if (!q.empty())
              break;
          }
          std::unique_lock lck(mtx);
          cv.wait(lck, [this] { return !q.empty(); });
          auto task = std::move(q.front());
          q.pop_front();
          lck.unlock();
          co_spawn(std::move(task));
        }
      });
    }
  };
  std::vector<Worker> workers(std::thread::hardware_concurrency());

  for (std::size_t i = 0; i < workers.size(); ++i) {
    workers[i].start(i);
  }
  co_await co_await stdio().putline("start loop"s );
  std::size_t i = 0;
  while (true) {
    if (auto income = co_await listener_accept(listener)) [[likely]] {
      workers[i].spawn(server.handle_http(std::move(*income)));
      ++i;
      if (i >= workers.size()) {
        i = 0;
      }
    }
  }
  co_return {};
}

int main(int argc, char **argv) {
  std::string serveAt = "0.0.0.0:8080";
  std::string confPath = "cfg/conf.json";
  if (argc > 1) {
    serveAt = argv[1];
    confPath = argv[2];
  } 
  co_main(amain(serveAt,confPath));
  return 0;
}
