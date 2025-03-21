#include "sqlMap.h"
#include <cstring>
//#include <mysql/field_types.h>
#include <mysql/mysql.h>
#include <string>
#include <vector>

int insertMAADailyTaskPlan(
    MYSQL *conn, const std::vector<MAADailyTaskPlan> &dailyTaskPlanList) {
  const char *query = "INSERT INTO MAADailyTaskPlan (planID,userID, deviceID, "
                      "dailyTaskStrategy, dailyTaskTime) VALUES (?,?, ?, ?, ?)";
  MYSQL_STMT *stmt = mysql_stmt_init(conn);
  if (!stmt) {
    return -1;
  }
  if (mysql_stmt_prepare(stmt, query, strlen(query))) {
    mysql_stmt_close(stmt);
    return -1;
  }
  MYSQL_BIND bind[5];
  memset(bind, 0, sizeof(bind));
  bind[0].buffer_type = MYSQL_TYPE_STRING;
  bind[1].buffer_type = MYSQL_TYPE_STRING;
  bind[2].buffer_type = MYSQL_TYPE_STRING;
  bind[3].buffer_type = MYSQL_TYPE_STRING;
  bind[4].buffer_type = MYSQL_TYPE_STRING;
  for (const auto &dailyTaskPlan : dailyTaskPlanList) {
    bind[0].buffer = (char *)dailyTaskPlan.planID.c_str();
    bind[0].buffer_length = dailyTaskPlan.planID.length();
    bind[1].buffer = (char *)dailyTaskPlan.userID.c_str();
    bind[1].buffer_length = dailyTaskPlan.userID.length();
    bind[2].buffer = (char *)dailyTaskPlan.deviceID.c_str();
    bind[2].buffer_length = dailyTaskPlan.deviceID.length();
    bind[3].buffer = (char *)dailyTaskPlan.dailyTaskStrategy.c_str();
    bind[3].buffer_length = dailyTaskPlan.dailyTaskStrategy.length();
    bind[4].buffer = (char *)dailyTaskPlan.dailyTaskTime.c_str();
    bind[4].buffer_length = dailyTaskPlan.dailyTaskTime.length();
    if (mysql_stmt_bind_param(stmt, bind)) {
      mysql_stmt_close(stmt);
      return -1;
    }
    if (mysql_stmt_execute(stmt)) {
      mysql_stmt_close(stmt);
      return -1;
    }
  }
  mysql_stmt_close(stmt);
  return 0;
}

int insertMAAQucikTask(MYSQL *conn,
                       const std::vector<MAAQucikTask> &quickTaskList) {
  const char *query =
      "INSERT INTO MAAQucikTask (taskID, userID, deviceID, taskCommitTime, "
      "taskStartTime, taskIsFinish, taskActions) VALUES (?, ?, ?, ?, ?, ?, ?)";
  MYSQL_STMT *stmt = mysql_stmt_init(conn);
  if (!stmt) {
    return -1;
  }
  if (mysql_stmt_prepare(stmt, query, strlen(query))) {
    mysql_stmt_close(stmt);
    return -1;
  }
  MYSQL_BIND bind[7];
  memset(bind, 0, sizeof(bind));
  bind[0].buffer_type = MYSQL_TYPE_STRING;
  bind[1].buffer_type = MYSQL_TYPE_STRING;
  bind[2].buffer_type = MYSQL_TYPE_STRING;
  bind[3].buffer_type = MYSQL_TYPE_STRING;
  bind[4].buffer_type = MYSQL_TYPE_STRING;
  bind[5].buffer_type = MYSQL_TYPE_STRING;
  bind[6].buffer_type = MYSQL_TYPE_STRING;
  for (const auto &quickTask : quickTaskList) {
    bind[0].buffer = (char *)quickTask.taskID.c_str();
    bind[0].buffer_length = quickTask.taskID.length();
    bind[1].buffer = (char *)quickTask.userID.c_str();
    bind[1].buffer_length = quickTask.userID.length();
    bind[2].buffer = (char *)quickTask.deviceID.c_str();
    bind[2].buffer_length = quickTask.deviceID.length();
    bind[3].buffer = (char *)quickTask.taskCommitTime.c_str();
    bind[3].buffer_length = quickTask.taskCommitTime.length();
    bind[4].buffer = (char *)quickTask.taskStartTime.c_str();
    bind[4].buffer_length = quickTask.taskStartTime.length();
    bind[5].buffer = (char *)quickTask.taskIsFinish.c_str();
    bind[5].buffer_length = quickTask.taskIsFinish.length();
    bind[6].buffer = (char *)quickTask.taskActions.c_str();
    bind[6].buffer_length = quickTask.taskActions.length();
    if (mysql_stmt_bind_param(stmt, bind)) {
      mysql_stmt_close(stmt);
      return -1;
    }
    if (mysql_stmt_execute(stmt)) {
      mysql_stmt_close(stmt);
      return -1;
    }
  }
  mysql_stmt_close(stmt);
  return 0;
}

int insertMAAAction(MYSQL *conn, const std::vector<MAAAction> &actionList) {
  const char *query = "INSERT INTO MAAAction (taskID, actionID, "
                      "actionIsFinish) VALUES (?, ?, ?)";
  MYSQL_STMT *stmt = mysql_stmt_init(conn);
  if (!stmt) {
    return -1;
  }
  if (mysql_stmt_prepare(stmt, query, strlen(query))) {
    mysql_stmt_close(stmt);
    return -1;
  }
  MYSQL_BIND bind[3];
  memset(bind, 0, sizeof(bind));
  bind[0].buffer_type = MYSQL_TYPE_STRING;
  bind[1].buffer_type = MYSQL_TYPE_STRING;
  bind[2].buffer_type = MYSQL_TYPE_STRING;
  for (const auto &action : actionList) {
    bind[0].buffer = (char *)action.taskID.c_str();
    bind[0].buffer_length = action.taskID.length();
    bind[1].buffer = (char *)action.actionID.c_str();
    bind[1].buffer_length = action.actionID.length();
    bind[2].buffer = (char *)action.actionIsFinish.c_str();
    bind[2].buffer_length = action.actionIsFinish.length();
    if (mysql_stmt_bind_param(stmt, bind)) {
      mysql_stmt_close(stmt);
      return -1;
    }
    if (mysql_stmt_execute(stmt)) {
      mysql_stmt_close(stmt);
      return -1;
    }
  }
  mysql_stmt_close(stmt);
  return 0;
}

int insertMAAUser(MYSQL *conn, const std::vector<MAAUser> &userList) {
  const char *query =
      "INSERT INTO MAAUser (userID, deviceID, nextDailyTaskTime, "
      "dailytaskStartTime, dailytaskEndTime, dailyTaskID) VALUES (?, ?, ?, ?, ?, ?)";
  MYSQL_STMT *stmt = mysql_stmt_init(conn);
  if (!stmt) {
    return -1;
  }
  if (mysql_stmt_prepare(stmt, query, strlen(query))) {
    mysql_stmt_close(stmt);
    return -1;
  }
  MYSQL_BIND bind[6];
  memset(bind, 0, sizeof(bind));
  bind[0].buffer_type = MYSQL_TYPE_STRING;
  bind[1].buffer_type = MYSQL_TYPE_STRING;
  bind[2].buffer_type = MYSQL_TYPE_STRING;
  bind[3].buffer_type = MYSQL_TYPE_STRING;
  bind[4].buffer_type = MYSQL_TYPE_STRING;
  bind[5].buffer_type = MYSQL_TYPE_STRING;
  for (const auto &user : userList) {
    bind[0].buffer = (char *)user.userID.c_str();
    bind[0].buffer_length = user.userID.length();
    bind[1].buffer = (char *)user.deviceID.c_str();
    bind[1].buffer_length = user.deviceID.length();
    bind[2].buffer = (char *)user.nextDailyTaskTime.c_str();
    bind[2].buffer_length = user.nextDailyTaskTime.length();
    bind[3].buffer = (char *)user.taskStartTime.c_str();
    bind[3].buffer_length = user.taskStartTime.length();
    bind[4].buffer = (char *)user.taskEndTime.c_str();
    bind[4].buffer_length = user.taskEndTime.length();
    bind[5].buffer = (char *)user.dailyTaskID.c_str();
    bind[5].buffer_length = user.dailyTaskID.length();
    if (mysql_stmt_bind_param(stmt, bind)) {
      mysql_stmt_close(stmt);
      return -1;
    }
    if (mysql_stmt_execute(stmt)) {
      mysql_stmt_close(stmt);
      return -1;
    }
  }
  mysql_stmt_close(stmt);
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

std::vector<MAAUser> queryMAAUserInfo(MYSQL *conn, std::string userID,
                                      std::string deviceID) {
  std::string sql =
      "SELECT userID,deviceID FROM MAAUser WHERE userID = ? AND deviceID = ?";
  MYSQL_STMT *stmt = mysql_stmt_init(conn);
  if (!stmt) {
    return {};
  }
  if (mysql_stmt_prepare(stmt, sql.c_str(), sql.size())) {
    mysql_stmt_close(stmt);
    return {};
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
    return {};
  }

  if (mysql_stmt_execute(stmt)) {
    mysql_stmt_close(stmt);
    return {};
  }

  MYSQL_RES *res = mysql_stmt_result_metadata(stmt);
  if (!res) {
    mysql_stmt_close(stmt);
    return {};
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
    return {};
  }

  std::vector<MAAUser> users;
  while (mysql_stmt_fetch(stmt) == 0) {
    for (auto &v : vals) {
      v.resize(strlen(v.c_str()));
    }
    users.push_back(MAAUser{
        .userID = vals[0],
        .deviceID = vals[1],
    });
  }
  mysql_free_result(res);
  mysql_stmt_close(stmt);
  return users;
}

std::vector<MAAUser> queryMAAUserTaskStatus(MYSQL *conn, std::string userID,
                                            std::string deviceID) {
  std::string sql = "SELECT nextDailyTaskTime,dailyTaskID FROM MAAUser WHERE "
                    "userID = ? AND deviceID = ?";
  MYSQL_STMT *stmt = mysql_stmt_init(conn);
  if (!stmt) {
    return {};
  }
  if (mysql_stmt_prepare(stmt, sql.c_str(), sql.size())) {
    mysql_stmt_close(stmt);
    return {};
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
    return {};
  }

  if (mysql_stmt_execute(stmt)) {
    mysql_stmt_close(stmt);
    return {};
  }

  MYSQL_RES *res = mysql_stmt_result_metadata(stmt);
  if (!res) {
    mysql_stmt_close(stmt);
    return {};
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
    mysql_free_result(res);
    mysql_stmt_close(stmt);
    return {};
  }

  std::vector<MAAUser> users;
  while (mysql_stmt_fetch(stmt) == 0) {
    for (auto &v : vals) {
      v.resize(strlen(v.c_str()));
    }
    users.push_back(
        MAAUser{.nextDailyTaskTime = vals[0], .dailyTaskID = vals[1]});
  }
  mysql_free_result(res);
  mysql_stmt_close(stmt);
  return users;
}

std::vector<MAADailyTaskPlan>
queryMAAUserStrategy(MYSQL *conn, std::string userID, std::string deviceID) {
  std::string sql = "SELECT planID,dailyTaskStrategy,dailyTaskTime FROM "
                    "MAADailyTaskPlan WHERE "
                    "userID = ? AND deviceID = ?";
  MYSQL_STMT *stmt = mysql_stmt_init(conn);
  if (!stmt) {
    return {};
  }
  if (mysql_stmt_prepare(stmt, sql.c_str(), sql.size())) {
    mysql_stmt_close(stmt);
    return {};
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
    return {};
  }

  if (mysql_stmt_execute(stmt)) {
    mysql_stmt_close(stmt);
    return {};
  }

  MYSQL_RES *res = mysql_stmt_result_metadata(stmt);
  if (!res) {
    mysql_stmt_close(stmt);
    return {};
  }

  MYSQL_BIND resultBind[3];
  memset(resultBind, 0, sizeof(resultBind));

  std::vector<std::string> vals;
  vals.reserve(3);
  for (int i = 0; i < 3; i++) {
    resultBind[i].buffer_type = MYSQL_TYPE_STRING;
    // 先为每个字符串分配固定大小
    vals.push_back(std::string(1024, '\0'));
    resultBind[i].buffer = (char *)vals.back().data();
    // 指定缓冲区大小
    resultBind[i].buffer_length = 1024;
    resultBind[i].is_null = nullptr;
    resultBind[i].length = nullptr;
  }

  if (mysql_stmt_bind_result(stmt, resultBind)) {
    mysql_free_result(res);
    mysql_stmt_close(stmt);
    return {};
  }

  std::vector<MAADailyTaskPlan> strategies;
  while (mysql_stmt_fetch(stmt) == 0) {
    for (auto &v : vals) {
      v.resize(strlen(v.c_str()));
    }
    strategies.push_back(MAADailyTaskPlan{.planID = vals[0],
                                          .dailyTaskStrategy = vals[1],
                                          .dailyTaskTime = vals[2]});
  }
  mysql_free_result(res);
  mysql_stmt_close(stmt);

  return strategies;
}

std::vector<MAAUser> queryMAAUserAllInfo(MYSQL *conn, std::string userID,
                                         std::string deviceID) {
  std::string sql =
      "SELECT "
      "userID,deviceID,nextDailyTaskTime,taskStartTime,taskEndTime,dailyTaskID "
      "FROM MAAUser WHERE userID = ? AND deviceID = ?";
  MYSQL_STMT *stmt = mysql_stmt_init(conn);
  if (!stmt) {
    return {};
  }
  if (mysql_stmt_prepare(stmt, sql.c_str(), sql.size())) {
    mysql_stmt_close(stmt);
    return {};
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
    return {};
  }

  if (mysql_stmt_execute(stmt)) {
    mysql_stmt_close(stmt);
    return {};
  }

  MYSQL_RES *res = mysql_stmt_result_metadata(stmt);
  if (!res) {
    mysql_stmt_close(stmt);
    return {};
  }

  MYSQL_BIND resultBind[6];
  memset(resultBind, 0, sizeof(resultBind));

  std::vector<std::string> vals;
  vals.reserve(6);
  for (int i = 0; i < 6; i++) {
    resultBind[i].buffer_type = MYSQL_TYPE_STRING;
    // 先为每个字符串分配固定大小
    vals.push_back(std::string(1024, '\0'));
    resultBind[i].buffer = (char *)vals.back().data();
    // 指定缓冲区大小
    resultBind[i].buffer_length = 1024;
    resultBind[i].is_null = nullptr;
    resultBind[i].length = nullptr;
  }

  if (mysql_stmt_bind_result(stmt, resultBind)) {
    mysql_free_result(res);
    mysql_stmt_close(stmt);
    return {};
  }

  std::vector<MAAUser> users;
  while (mysql_stmt_fetch(stmt) == 0) {
    for (auto &v : vals) {
      v.resize(strlen(v.c_str()));
    }
    users.push_back(MAAUser{.userID = vals[0],
                            .deviceID = vals[1],
                            .nextDailyTaskTime = vals[2],
                            .taskStartTime = vals[3],
                            .taskEndTime = vals[4],
                            .dailyTaskID = vals[5]});
  }
  mysql_free_result(res);
  mysql_stmt_close(stmt);
  return users;
}

std::vector<MAAQucikTask> queryMAAQuickTask(MYSQL *conn, std::string userID,
                                            std::string deviceID,
                                            std::string taskIsFinish) {
  const char *query =
      "SELECT taskID,taskCommitTime,taskStartTime,taskIsFinish,taskActions "
      "FROM MAAQucikTask WHERE userID = ? AND deviceID = ? AND taskIsFinish = "
      "?";
  MYSQL_STMT *stmt = mysql_stmt_init(conn);
  if (!stmt) {
    return {};
  }
  if (mysql_stmt_prepare(stmt, query, strlen(query))) {
    mysql_stmt_close(stmt);
    return {};
  }
  MYSQL_BIND bind[3];
  memset(bind, 0, sizeof(bind));
  bind[0].buffer_type = MYSQL_TYPE_STRING;
  bind[1].buffer_type = MYSQL_TYPE_STRING;
  bind[2].buffer_type = MYSQL_TYPE_STRING;
  bind[0].buffer = (char *)userID.c_str();
  bind[0].buffer_length = userID.length();
  bind[1].buffer = (char *)deviceID.c_str();
  bind[1].buffer_length = deviceID.length();
  bind[2].buffer = (char *)taskIsFinish.c_str();
  bind[2].buffer_length = taskIsFinish.length();
  if (mysql_stmt_bind_param(stmt, bind)) {
    mysql_stmt_close(stmt);
    return {};
  }
  if (mysql_stmt_execute(stmt)) {
    mysql_stmt_close(stmt);
    return {};
  }
  MYSQL_RES *res = mysql_stmt_result_metadata(stmt);
  if (!res) {
    mysql_stmt_close(stmt);
    return {};
  }
  MYSQL_BIND resultBind[5];
  memset(resultBind, 0, sizeof(resultBind));
  std::vector<std::string> vals;
  vals.reserve(5);
  for (int i = 0; i < 5; i++) {
    resultBind[i].buffer_type = MYSQL_TYPE_STRING;
    vals.push_back(std::string(1024, '\0'));
    resultBind[i].buffer = (char *)vals.back().data();
    resultBind[i].buffer_length = 1024;
    resultBind[i].is_null = nullptr;
    resultBind[i].length = nullptr;
  }
  if (mysql_stmt_bind_result(stmt, resultBind)) {
    mysql_free_result(res);
    mysql_stmt_close(stmt);
    return {};
  }
  std::vector<MAAQucikTask> quickTasks;
  while (mysql_stmt_fetch(stmt) == 0) {
    for (auto &v : vals) {
      v.resize(strlen(v.c_str()));
    }
    quickTasks.push_back(MAAQucikTask{.taskID = vals[0],
                                      .taskCommitTime = vals[1],
                                      .taskStartTime = vals[2],
                                      .taskIsFinish = vals[3],
                                      .taskActions = vals[4]});
  }
  mysql_free_result(res);
  mysql_stmt_close(stmt);
  return quickTasks;
}

bool updateMAAAction(MYSQL *conn, std::string actionID,
                     std::string actionIsFinish) {
  const char *query = "UPDATE MAAAction SET actionIsFinish = ? WHERE actionID = ?";
  MYSQL_STMT *stmt = mysql_stmt_init(conn);
  if (!stmt) {
    return false;
  }
  if (mysql_stmt_prepare(stmt, query, strlen(query))) {
    mysql_stmt_close(stmt);
    return false;
  }
  MYSQL_BIND bind[2];
  memset(bind, 0, sizeof(bind));
  bind[0].buffer_type = MYSQL_TYPE_STRING;
  bind[1].buffer_type = MYSQL_TYPE_STRING;
  bind[0].buffer = (char *)actionIsFinish.c_str();
  bind[0].buffer_length = actionIsFinish.length();
  bind[1].buffer = (char *)actionID.c_str();
  bind[1].buffer_length = actionID.length();
  if (mysql_stmt_bind_param(stmt, bind)) {
    mysql_stmt_close(stmt);
    return false;
  }
  if (mysql_stmt_execute(stmt)) {
    mysql_stmt_close(stmt);
    return false;
  }
  if (mysql_stmt_affected_rows(stmt) == 0) {
        mysql_stmt_close(stmt);
        return false;
    }
  mysql_stmt_close(stmt);
  return true;
}

std::vector<MAAAction> queryMAAAction(MYSQL *conn, std::string actionID){
  const char *query = "SELECT taskID,actionIsFinish FROM MAAAction WHERE actionID = ?";
  MYSQL_STMT *stmt = mysql_stmt_init(conn);
  if (!stmt) {
    return {};
  }
  if (mysql_stmt_prepare(stmt, query, strlen(query))) {
    mysql_stmt_close(stmt);
    return {};
  }
  MYSQL_BIND bind[1];
  memset(bind, 0, sizeof(bind));
  bind[0].buffer_type = MYSQL_TYPE_STRING;
  bind[0].buffer = (char *)actionID.c_str();
  bind[0].buffer_length = actionID.length();
  if (mysql_stmt_bind_param(stmt, bind)) {
    mysql_stmt_close(stmt);
    return {};
  }
  if (mysql_stmt_execute(stmt)) {
    mysql_stmt_close(stmt);
    return {};
  }
  MYSQL_RES *res = mysql_stmt_result_metadata(stmt);
  if (!res) {
    mysql_stmt_close(stmt);
    return {};
  }
  MYSQL_BIND resultBind[2];
  memset(resultBind, 0, sizeof(resultBind));
  std::vector<std::string> vals;
  vals.reserve(2);
  for (int i = 0; i < 2; i++) {
    resultBind[i].buffer_type = MYSQL_TYPE_STRING;
    vals.push_back(std::string(1024, '\0'));
    resultBind[i].buffer = (char *)vals.back().data();
    resultBind[i].buffer_length = 1024;
    resultBind[i].is_null = nullptr;
    resultBind[i].length = nullptr;
  }
  if (mysql_stmt_bind_result(stmt, resultBind)) {
    mysql_free_result(res);
    mysql_stmt_close(stmt);
    return {};
  }
  std::vector<MAAAction> actions;
  while (mysql_stmt_fetch(stmt) == 0) {
    for (auto &v : vals) {
      v.resize(strlen(v.c_str()));
    }
    actions.push_back(MAAAction{
                                .taskID = vals[0],
                                .actionIsFinish = vals[1]});
  }
  mysql_free_result(res);
  mysql_stmt_close(stmt);
  return actions;
}

std::vector<MAAAction> queryMAAAction(MYSQL *conn, std::string taskID, std::string actionIsFinish){
  const char *query = "SELECT actionID,actionIsFinish FROM MAAAction WHERE taskID = ? AND actionIsFinish = ?";
  MYSQL_STMT *stmt = mysql_stmt_init(conn);
  if (!stmt) {
    return {};
  }
  if (mysql_stmt_prepare(stmt, query, strlen(query))) {
    mysql_stmt_close(stmt);
    return {};
  }
  MYSQL_BIND bind[2];
  memset(bind, 0, sizeof(bind));
  bind[0].buffer_type = MYSQL_TYPE_STRING;
  bind[1].buffer_type = MYSQL_TYPE_STRING;
  bind[0].buffer = (char *)taskID.c_str();
  bind[0].buffer_length = taskID.length();
  bind[1].buffer = (char *)actionIsFinish.c_str();
  bind[1].buffer_length = actionIsFinish.length();
  if (mysql_stmt_bind_param(stmt, bind)) {
    mysql_stmt_close(stmt);
    return {};
  }
  if (mysql_stmt_execute(stmt)) {
    mysql_stmt_close(stmt);
    return {};
  }
  MYSQL_RES *res = mysql_stmt_result_metadata(stmt);
  if (!res) {
    mysql_stmt_close(stmt);
    return {};
  }
  MYSQL_BIND resultBind[2];
  memset(resultBind, 0, sizeof(resultBind));
  std::vector<std::string> vals;
  vals.reserve(2);
  for (int i = 0; i < 2; i++) {
    resultBind[i].buffer_type = MYSQL_TYPE_STRING;
    vals.push_back(std::string(1024, '\0'));
    resultBind[i].buffer = (char *)vals.back().data();
    resultBind[i].buffer_length = 1024;
    resultBind[i].is_null = nullptr;
    resultBind[i].length = nullptr;
  }
  if (mysql_stmt_bind_result(stmt, resultBind)) {
    mysql_free_result(res);
    mysql_stmt_close(stmt);
    return {};
  }
  std::vector<MAAAction> actions;
  while (mysql_stmt_fetch(stmt) == 0) {
    for (auto &v : vals) {
      v.resize(strlen(v.c_str()));
    }
    actions.push_back(MAAAction{
                                .actionID = vals[0],
                                .actionIsFinish = vals[1]});
  }
  mysql_free_result(res);
  mysql_stmt_close(stmt);
  return actions;
}

bool updateMAAQucikTask(MYSQL *conn, std::string taskID,std::unordered_map<std::string, std::string>& updateColMap){
  std::string sql = "UPDATE MAAQucikTask SET ";
  bool first = true;
  for (const auto &kv : updateColMap) {
    if (!first)
      sql += ", ";
    sql += kv.first + " = ?";
    first = false;
  }
  sql += " WHERE taskID = ?";

  MYSQL_STMT *stmt = mysql_stmt_init(conn);
  if (!stmt) {
    return false;
  }


  if (mysql_stmt_prepare(stmt, sql.c_str(), sql.size())) {
    mysql_stmt_close(stmt);
    return false;
  }

  size_t paramCount = updateColMap.size() + 1;
  std::vector<MYSQL_BIND> binds(paramCount);
  memset(binds.data(), 0, sizeof(MYSQL_BIND) * paramCount);

  int idx = 0;
  std::vector<std::string> vals; 
  vals.reserve(paramCount);
  for (const auto &kv : updateColMap) {
    vals.push_back(kv.second);
    binds[idx].buffer_type = MYSQL_TYPE_STRING;
    binds[idx].buffer = (char *)vals.back().c_str();
    binds[idx].buffer_length = vals.back().size();
    idx++;
  }

  vals.push_back(taskID);
  binds[idx].buffer_type = MYSQL_TYPE_STRING;
  binds[idx].buffer = (char *)vals.back().c_str();
  binds[idx].buffer_length = vals.back().size();
  idx++;

  if (mysql_stmt_bind_param(stmt, binds.data())) {
    mysql_stmt_close(stmt);
    return false;
  }

  if (mysql_stmt_execute(stmt)) {
    mysql_stmt_close(stmt);
    return false;
  }

  mysql_stmt_close(stmt);
  return true;
}

bool updateMAADailyTaskPlan(MYSQL *conn, std::string planID,std::string userID,std::string deviceID,std::unordered_map<std::string, std::string>& updateColMap){
  std::string sql = "UPDATE MAADailyTaskPlan SET ";
  bool first = true;
  for (const auto &kv : updateColMap) {
    if (!first)
      sql += ", ";
    sql += kv.first + " = ?";
    first = false;
  }
  sql += " WHERE planID = ? AND userID = ? AND deviceID = ?";

  MYSQL_STMT *stmt = mysql_stmt_init(conn);
  if (!stmt) {
    return false;
  }

  if (mysql_stmt_prepare(stmt, sql.c_str(), sql.size())) {
    mysql_stmt_close(stmt);
    return false;
  }

  size_t paramCount = updateColMap.size() + 3;
  std::vector<MYSQL_BIND> binds(paramCount);
  memset(binds.data(), 0, sizeof(MYSQL_BIND) * paramCount);

  int idx = 0;
  std::vector<std::string> vals; 
  vals.reserve(paramCount);
  for (const auto &kv : updateColMap) {
    vals.push_back(kv.second);
    binds[idx].buffer_type = MYSQL_TYPE_STRING;
    binds[idx].buffer = (char *)vals.back().c_str();
    binds[idx].buffer_length = vals.back().size();
    idx++;
  }

  // 绑定 planID, userID, deviceID
  vals.push_back(planID);
  binds[idx].buffer_type = MYSQL_TYPE_STRING;
  binds[idx].buffer = (char *)vals.back().c_str();
  binds[idx].buffer_length = vals.back().size();
  idx++;

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
  
  if (mysql_stmt_execute(stmt)) {
    mysql_stmt_close(stmt);
    return false;
  }

  mysql_stmt_close(stmt);
  return true;
}