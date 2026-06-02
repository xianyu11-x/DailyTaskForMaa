#include "sqlMap.h"
#include <cstring>
//#include <mysql/field_types.h>
#include <mysql/mysql.h>
#include <algorithm>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

constexpr unsigned long kMinResultBufferSize = 256;
constexpr unsigned long kMaxInitialResultBufferSize = 4096;
using MysqlBindBool =
    std::remove_pointer_t<decltype(std::declval<MYSQL_BIND>().is_null)>;

struct QueryResultBuffers {
  std::vector<MYSQL_BIND> binds;
  std::vector<std::vector<char>> buffers;
  std::vector<unsigned long> lengths;
  std::unique_ptr<MysqlBindBool[]> isNull;
  std::unique_ptr<MysqlBindBool[]> errors;
};

QueryResultBuffers makeResultBuffers(MYSQL_RES *metadata) {
  const auto fieldCount = mysql_num_fields(metadata);
  MYSQL_FIELD *fields = mysql_fetch_fields(metadata);
  QueryResultBuffers result;
  result.binds.resize(fieldCount);
  result.buffers.resize(fieldCount);
  result.lengths.resize(fieldCount);
  result.isNull = std::make_unique<MysqlBindBool[]>(fieldCount);
  result.errors = std::make_unique<MysqlBindBool[]>(fieldCount);

  for (unsigned int i = 0; i < fieldCount; ++i) {
    const unsigned long fieldLength =
        fields ? fields[i].length : kMinResultBufferSize;
    const unsigned long cappedFieldLength =
        std::min(fieldLength, kMaxInitialResultBufferSize - 1);
    const unsigned long bufferLength =
        std::max(cappedFieldLength + 1, kMinResultBufferSize);
    result.buffers[i].assign(bufferLength, '\0');
    result.binds[i].buffer_type = MYSQL_TYPE_STRING;
    result.binds[i].buffer = result.buffers[i].data();
    result.binds[i].buffer_length = bufferLength;
    result.binds[i].length = &result.lengths[i];
    result.binds[i].is_null = &result.isNull[i];
    result.binds[i].error = &result.errors[i];
  }

  return result;
}

std::string resultString(const QueryResultBuffers &result, std::size_t index) {
  if (index >= result.buffers.size() || result.isNull[index]) {
    return "";
  }
  const auto length = std::min<unsigned long>(
      result.lengths[index], result.buffers[index].size());
  return std::string(result.buffers[index].data(), length);
}

bool bindResultBuffers(MYSQL_STMT *stmt, MYSQL_RES *metadata,
                       QueryResultBuffers &result) {
  result = makeResultBuffers(metadata);
  return mysql_stmt_bind_result(stmt, result.binds.data()) == 0;
}

bool fetchNextRow(MYSQL_STMT *stmt, QueryResultBuffers &result) {
  std::fill(result.errors.get(), result.errors.get() + result.binds.size(),
            false);
  int fetchStatus = mysql_stmt_fetch(stmt);
  if (fetchStatus == MYSQL_NO_DATA) {
    return false;
  }
  if (fetchStatus != 0 && fetchStatus != MYSQL_DATA_TRUNCATED) {
    return false;
  }

  for (std::size_t i = 0; i < result.binds.size(); ++i) {
    if (result.errors[i] && !result.isNull[i] &&
        result.lengths[i] >= result.buffers[i].size()) {
      result.buffers[i].assign(result.lengths[i] + 1, '\0');
      result.binds[i].buffer = result.buffers[i].data();
      result.binds[i].buffer_length = result.buffers[i].size();
      mysql_stmt_fetch_column(stmt, &result.binds[i], i, 0);
    }
  }

  return true;
}

} // namespace

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

int insertMAAQuickTask(MYSQL *conn,
                       const std::vector<MAAQuickTask> &quickTaskList) {
  const char *query =
      "INSERT INTO MAAQuickTask (taskID, userID, deviceID, taskCommitTime, "
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
    bind[3].buffer = (char *)user.dailyTaskStartTime.c_str();
    bind[3].buffer_length = user.dailyTaskStartTime.length();
    bind[4].buffer = (char *)user.dailyTaskEndTime.c_str();
    bind[4].buffer_length = user.dailyTaskEndTime.length();
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
  sql += " WHERE userID = ? AND deviceID = ?";

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

  QueryResultBuffers result;
  if (!bindResultBuffers(stmt, res, result)) {
    mysql_free_result(res);
    mysql_stmt_close(stmt);
    return {};
  }

  std::vector<MAAUser> users;
  while (fetchNextRow(stmt, result)) {
    users.push_back(MAAUser{
        .userID = resultString(result, 0),
        .deviceID = resultString(result, 1),
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

  QueryResultBuffers result;
  if (!bindResultBuffers(stmt, res, result)) {
    mysql_free_result(res);
    mysql_stmt_close(stmt);
    return {};
  }

  std::vector<MAAUser> users;
  while (fetchNextRow(stmt, result)) {
    users.push_back(
        MAAUser{.nextDailyTaskTime = resultString(result, 0),
                .dailyTaskID = resultString(result, 1)});
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

  QueryResultBuffers result;
  if (!bindResultBuffers(stmt, res, result)) {
    mysql_free_result(res);
    mysql_stmt_close(stmt);
    return {};
  }

  std::vector<MAADailyTaskPlan> strategies;
  while (fetchNextRow(stmt, result)) {
    strategies.push_back(MAADailyTaskPlan{
        .planID = resultString(result, 0),
        .dailyTaskStrategy = resultString(result, 1),
        .dailyTaskTime = resultString(result, 2)});
  }
  mysql_free_result(res);
  mysql_stmt_close(stmt);

  return strategies;
}

std::vector<MAAUser> queryMAAUserAllInfo(MYSQL *conn, std::string userID,
                                         std::string deviceID) {
  std::string sql =
      "SELECT "
      "userID,deviceID,nextDailyTaskTime,dailytaskStartTime,dailytaskEndTime,dailyTaskID "
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

  QueryResultBuffers result;
  if (!bindResultBuffers(stmt, res, result)) {
    mysql_free_result(res);
    mysql_stmt_close(stmt);
    return {};
  }

  std::vector<MAAUser> users;
  while (fetchNextRow(stmt, result)) {
    users.push_back(MAAUser{.userID = resultString(result, 0),
                            .deviceID = resultString(result, 1),
                            .nextDailyTaskTime = resultString(result, 2),
                            .dailyTaskStartTime = resultString(result, 3),
                            .dailyTaskEndTime = resultString(result, 4),
                            .dailyTaskID = resultString(result, 5)});
  }
  mysql_free_result(res);
  mysql_stmt_close(stmt);
  return users;
}

std::vector<MAAQuickTask> queryMAAQuickTask(MYSQL *conn, std::string userID,
                                            std::string deviceID,
                                            std::string taskIsFinish) {
  const char *query =
      "SELECT taskID,taskCommitTime,taskStartTime,taskIsFinish,taskActions "
      "FROM MAAQuickTask WHERE userID = ? AND deviceID = ? AND taskIsFinish = "
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
  QueryResultBuffers result;
  if (!bindResultBuffers(stmt, res, result)) {
    mysql_free_result(res);
    mysql_stmt_close(stmt);
    return {};
  }
  std::vector<MAAQuickTask> quickTasks;
  while (fetchNextRow(stmt, result)) {
    quickTasks.push_back(MAAQuickTask{.taskID = resultString(result, 0),
                                      .taskCommitTime = resultString(result, 1),
                                      .taskStartTime = resultString(result, 2),
                                      .taskIsFinish = resultString(result, 3),
                                      .taskActions = resultString(result, 4)});
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
  QueryResultBuffers result;
  if (!bindResultBuffers(stmt, res, result)) {
    mysql_free_result(res);
    mysql_stmt_close(stmt);
    return {};
  }
  std::vector<MAAAction> actions;
  while (fetchNextRow(stmt, result)) {
    actions.push_back(MAAAction{
                                .taskID = resultString(result, 0),
                                .actionIsFinish = resultString(result, 1)});
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
  QueryResultBuffers result;
  if (!bindResultBuffers(stmt, res, result)) {
    mysql_free_result(res);
    mysql_stmt_close(stmt);
    return {};
  }
  std::vector<MAAAction> actions;
  while (fetchNextRow(stmt, result)) {
    actions.push_back(MAAAction{
                                .actionID = resultString(result, 0),
                                .actionIsFinish = resultString(result, 1)});
  }
  mysql_free_result(res);
  mysql_stmt_close(stmt);
  return actions;
}

bool updateMAAQuickTask(MYSQL *conn, std::string taskID,std::unordered_map<std::string, std::string>& updateColMap){
  std::string sql = "UPDATE MAAQuickTask SET ";
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

bool deleteMAADailyTaskPlan(MYSQL *conn, std::string planID){
  std::string sql = "DELETE FROM MAADailyTaskPlan WHERE planID = ?";
  MYSQL_STMT *stmt = mysql_stmt_init(conn);
  if (!stmt) {
    return false;
  }
  if (mysql_stmt_prepare(stmt, sql.c_str(), sql.size())) {
    mysql_stmt_close(stmt);
    return false;
  }
  MYSQL_BIND bind[1];
  memset(bind, 0, sizeof(bind));
  bind[0].buffer_type = MYSQL_TYPE_STRING;
  bind[0].buffer = (char *)planID.c_str();
  bind[0].buffer_length = planID.length();
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
