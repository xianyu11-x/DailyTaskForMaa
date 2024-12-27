#include "sqlMap.h"
#include <cstring>
#include <string>
#include <vector>

int insertMAAUserInit(MYSQL *conn, std::string userID, std::string deviceID,
                      std::string taskStrategy, std::string dailyTaskTime,
                      std::string curTaskDefaultTime) {
  const char *query = "INSERT INTO MAAUser (UserID, DeviceID, taskStrategy, "
                      "dailyTaskTime, taskStartTime) VALUES (?, ?, ?, ?, ?)";
  MYSQL_STMT *stmt = mysql_stmt_init(conn);
  if (!stmt) {
    return -1;
  }
  if (mysql_stmt_prepare(stmt, query, strlen(query))) {
    return -1;
  }
  MYSQL_BIND bind[5];
  memset(bind, 0, sizeof(bind));
  bind[0].buffer_type = MYSQL_TYPE_STRING;
  bind[0].buffer = (char *)userID.c_str();
  bind[0].buffer_length = userID.length();

  bind[1].buffer_type = MYSQL_TYPE_STRING;
  bind[1].buffer = (char *)deviceID.c_str();
  bind[1].buffer_length = deviceID.length();

  bind[2].buffer_type = MYSQL_TYPE_STRING;
  bind[2].buffer = (char *)taskStrategy.c_str();
  bind[2].buffer_length = taskStrategy.length();

  bind[3].buffer_type = MYSQL_TYPE_STRING;
  bind[3].buffer = (char *)dailyTaskTime.c_str();
  bind[3].buffer_length = dailyTaskTime.length();

  bind[4].buffer_type = MYSQL_TYPE_STRING;
  bind[4].buffer = (char *)curTaskDefaultTime.c_str();
  bind[4].buffer_length = curTaskDefaultTime.length();

  if (mysql_stmt_bind_param(stmt, bind)) {
    return -1;
  }

  if (mysql_stmt_execute(stmt)) {
    return -1;
  }
  return 0;
}

bool updateMAAUser(MYSQL *conn, std::string userID, std::string deviceID,
                   std::unordered_map<std::string, std::string> &updateColMap) {
  std::string sql = "UPDATE MAAUser SET ";
  bool first = true;
  for (const auto &kv : updateColMap) {
    if (!first)
      sql += ", ";
    sql += kv.first + " = ?";
    first = false;
  }
  sql += " WHERE UserID = ? AND DeviceID = ?";

  // 初始化预处理语句
  MYSQL_STMT *stmt = mysql_stmt_init(conn);
  if (!stmt) {
    return false;
  }

  // 准备预处理语句
  if (mysql_stmt_prepare(stmt, sql.c_str(), sql.size())) {
    mysql_stmt_close(stmt);
    return false;
  }

  // 绑定参数
  size_t paramCount = updateColMap.size() + 2;
  std::vector<MYSQL_BIND> binds(paramCount);
  memset(binds.data(), 0, sizeof(MYSQL_BIND) * paramCount);

  // 为每个列占位符绑定数据
  int idx = 0;
  std::vector<std::string> vals; // 存放实际字符串，防止被回收
  vals.reserve(paramCount);
  for (const auto &kv : updateColMap) {
    vals.push_back(kv.second);
    binds[idx].buffer_type = MYSQL_TYPE_STRING;
    binds[idx].buffer = (char *)vals.back().c_str();
    binds[idx].buffer_length = vals.back().size();
    idx++;
  }

  // 绑定 userID, deviceID
  vals.push_back(userID);
  binds[idx].buffer_type = MYSQL_TYPE_STRING;
  binds[idx].buffer = (char *)vals.back().c_str();
  binds[idx].buffer_length = vals.back().size();
  idx++;

  vals.push_back(deviceID);
  binds[idx].buffer_type = MYSQL_TYPE_STRING;
  binds[idx].buffer = (char *)vals.back().c_str();
  binds[idx].buffer_length = vals.back().size();
  idx++;

  if (mysql_stmt_bind_param(stmt, binds.data())) {
    mysql_stmt_close(stmt);
    return false;
  }

  // 执行预处理语句
  if (mysql_stmt_execute(stmt)) {
    mysql_stmt_close(stmt);
    return false;
  }

  // 关闭预处理语句
  mysql_stmt_close(stmt);
  return true;
}

MAAUser queryMAAUserInfo(MYSQL *conn, std::string userID,
                         std::string deviceID) {
  std::string sql =
      "SELECT userID,deviceID FROM MAAUser WHERE userID = ? AND deviceID = ?";
  MYSQL_STMT *stmt = mysql_stmt_init(conn);
  if (!stmt) {
    return MAAUser();
  }
  if (mysql_stmt_prepare(stmt, sql.c_str(), sql.size())) {
    mysql_stmt_close(stmt);
    return MAAUser();
  }

  MYSQL_BIND bind[2];
  memset(bind, 0, sizeof(bind));

  bind[0].buffer_type = MYSQL_TYPE_STRING;
  bind[0].buffer = (char *)userID.c_str();
  bind[0].buffer_length = userID.size();

  bind[1].buffer_type = MYSQL_TYPE_STRING;
  bind[1].buffer = (char *)deviceID.c_str();
  bind[1].buffer_length = deviceID.size();

  if (mysql_stmt_bind_param(stmt, bind)) {
    mysql_stmt_close(stmt);
    return MAAUser();
  }

  if (mysql_stmt_execute(stmt)) {
    mysql_stmt_close(stmt);
    return MAAUser();
  }

  MYSQL_RES *res = mysql_stmt_result_metadata(stmt);
  if (!res) {
    mysql_stmt_close(stmt);
    return MAAUser();
  }

  MYSQL_BIND resultBind[2];
  memset(resultBind, 0, sizeof(resultBind));

  std::vector<std::string> vals;
  vals.reserve(2);
  for (int i = 0; i < 2; i++) {
    resultBind[i].buffer_type = MYSQL_TYPE_STRING;
    // 先为每个字符串分配固定大小
    vals.push_back(std::string(256, '\0'));
    resultBind[i].buffer = (char *)vals.back().data();
    // 指定缓冲区大小
    resultBind[i].buffer_length = 256;
    resultBind[i].is_null = nullptr;
    resultBind[i].length = nullptr;
  }
  if (mysql_stmt_bind_result(stmt, resultBind)) {
    mysql_stmt_close(stmt);
    return MAAUser();
  }

  if (mysql_stmt_fetch(stmt) == 1) {
    mysql_stmt_close(stmt);
    return MAAUser();
  } else {
    for (auto &v : vals) {
      v.resize(strlen(v.c_str()));
    }
  }

  mysql_stmt_close(stmt);
  return MAAUser{.userID = vals[0], .deviceID = vals[1]};
}

MAAUser queryMAAUserTaskStatus(MYSQL *conn, std::string userID,
                               std::string deviceID) {
  std::string sql = "SELECT dailyTaskTime,dailyTaskID FROM MAAUser WHERE "
                    "userID = ? AND deviceID = ?";
  MYSQL_STMT *stmt = mysql_stmt_init(conn);
  if (!stmt) {
    return MAAUser();
  }
  if (mysql_stmt_prepare(stmt, sql.c_str(), sql.size())) {
    mysql_stmt_close(stmt);
    return MAAUser();
  }
  MYSQL_BIND bind[2];
  memset(bind, 0, sizeof(bind));

  bind[0].buffer_type = MYSQL_TYPE_STRING;
  bind[0].buffer = (char *)userID.c_str();
  bind[0].buffer_length = userID.size();

  bind[1].buffer_type = MYSQL_TYPE_STRING;
  bind[1].buffer = (char *)deviceID.c_str();
  bind[1].buffer_length = deviceID.size();

  if (mysql_stmt_bind_param(stmt, bind)) {
    mysql_stmt_close(stmt);
    return MAAUser();
  }

  if (mysql_stmt_execute(stmt)) {
    mysql_stmt_close(stmt);
    return MAAUser();
  }

  MYSQL_RES *res = mysql_stmt_result_metadata(stmt);
  if (!res) {
    mysql_stmt_close(stmt);
    return MAAUser();
  }

  MYSQL_BIND resultBind[2];
  memset(resultBind, 0, sizeof(resultBind));

  std::vector<std::string> vals;
  vals.reserve(2);
  for (int i = 0; i < 2; i++) {
    resultBind[i].buffer_type = MYSQL_TYPE_STRING;
    // 先为每个字符串分配固定大小
    vals.push_back(std::string(256, '\0'));
    resultBind[i].buffer = (char *)vals.back().data();
    // 指定缓冲区大小
    resultBind[i].buffer_length = 256;
    resultBind[i].is_null = nullptr;
    resultBind[i].length = nullptr;
  }

  if (mysql_stmt_bind_result(stmt, resultBind)) {
    mysql_stmt_close(stmt);
    return MAAUser();
  }

 if (mysql_stmt_fetch(stmt) == 1) {
    mysql_stmt_close(stmt);
    return MAAUser();
  } else {
    for (auto &v : vals) {
      v.resize(strlen(v.c_str()));
    }
  }

  mysql_stmt_close(stmt);
  return MAAUser{.dailyTaskTime = vals[0], .dailyTaskID = vals[1]};
}