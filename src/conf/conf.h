#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct dbInfo {
  std::string url;
  std::string user;
  std::string password;
  std::string databaseName;
  int port;
  int maxConn;
};

class confManager {
public:
  static confManager *GetInstance();
  void init(const std::string &jsonStr);
  const dbInfo &getDbInfo() const { return db; }
  const int getBackendPort() const { return backendPort; }
  const std::vector<std::vector<std::string>> &getDefaultLevelList() const {
    return defaultLevelList;
  }
  const std::vector<std::string> &getDefaultDailyTaskTimeList() const {
    return defaultDailyTaskTimeList;
  }
  const std::string &getPermantLevelPath() const { return permanentLevelPath; }
private:
  confManager() = default;
  confManager(const confManager &) = delete;
  confManager &operator=(const confManager &) = delete;

  dbInfo db;
  int backendPort;
  std::vector<std::vector<std::string>> defaultLevelList;
  std::vector<std::string> defaultDailyTaskTimeList;
  std::string permanentLevelPath;
};