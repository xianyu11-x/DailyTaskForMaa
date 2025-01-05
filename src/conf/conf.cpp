#include "conf.h"
#include <string>
#include <vector>

confManager *confManager::GetInstance() {
  static confManager instance;
  return &instance;
}

void confManager::init(const std::string &jsonStr) {
  rapidjson::Document doc;
  doc.Parse(jsonStr.c_str());
  if (doc.HasParseError()) {
    throw std::runtime_error("parse json error");
  }

  if (doc.HasMember("mysql")) {
    const rapidjson::Value &db = doc["mysql"];
    if (db.HasMember("url")) {
      this->db.url = db["url"].GetString();
    }
    if (db.HasMember("user")) {
      this->db.user = db["user"].GetString();
    }
    if (db.HasMember("password")) {
      this->db.password = db["password"].GetString();
    }
    if (db.HasMember("database")) {
      this->db.databaseName = db["database"].GetString();
    }
    if (db.HasMember("port")) {
      this->db.port = db["port"].GetInt();
    }
    if (db.HasMember("connectionLimit")) {
      this->db.maxConn = db["connectionLimit"].GetInt();
    }
  }

  if (doc.HasMember("backendPort")) {
    this->backendPort = doc["backendPort"].GetInt();
  }

  if (doc.HasMember("permanentLevel")) {
    this->permanentLevelPath = doc["permanentLevel"].GetString();
  }

  if (doc.HasMember("defaultUserSettings")) {
    auto &defaultUserSettings = doc["defaultUserSettings"];
    if (defaultUserSettings.HasMember("dailyTaskTime")) {
      auto &dailyTaskTime = defaultUserSettings["dailyTaskTime"];
      for (auto &v : dailyTaskTime.GetArray()) {
        this->defaultDailyTaskTimeList.push_back(v.GetString());
      }
    }
    if (defaultUserSettings.HasMember("defaultLevel")) {
      for (int i = 1; i <= 7; i++) {
        std::vector<std::string> levelList;
        if (defaultUserSettings["defaultLevel"].HasMember(
                std::to_string(i).c_str())) {
          auto &level =
              defaultUserSettings["defaultLevel"][std::to_string(i).c_str()];
          for (auto &v : level.GetArray()) {
            levelList.push_back(v.GetString());
          }
        }
        this->defaultLevelList.push_back(levelList);
      }
    }
  }
}