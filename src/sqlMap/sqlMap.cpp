#include "sqlMap.h"
#include <cstring>
#include <string>
#include <vector>

int insertMAAUserInit(MYSQL *conn, std::string userID, std::string deviceID, std::string taskStrategy,std::string dailyTaskTime, std::string curTaskDefaultTime){
    const char* query = "INSERT INTO MAAUser (UserID, DeviceID, taskStrategy, dailyTaskTime, taskStartTime) VALUES (?, ?, ?, ?, ?)";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt){
        return -1;
    }
    if(mysql_stmt_prepare(stmt, query, strlen(query))){
        return -1;
    }
    MYSQL_BIND bind[5];
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)userID.c_str();
    bind[0].buffer_length = userID.length();

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (char*)deviceID.c_str();
    bind[1].buffer_length = deviceID.length();

    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (char*)taskStrategy.c_str();
    bind[2].buffer_length = taskStrategy.length();

    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = (char*)dailyTaskTime.c_str();
    bind[3].buffer_length = dailyTaskTime.length();

    bind[4].buffer_type = MYSQL_TYPE_STRING;
    bind[4].buffer = (char*)curTaskDefaultTime.c_str();
    bind[4].buffer_length = curTaskDefaultTime.length();

    if(mysql_stmt_bind_param(stmt, bind)){
        return -1;
    }

    if(mysql_stmt_execute(stmt)){
        return -1;
    }
    return 0;
}

bool updateMAAUser(MYSQL *conn, std::string userID, std::string deviceID,std::unordered_map<std::string, std::string>& updateColMap){
    std::string sql = "UPDATE MAAUser SET ";
    bool first = true;
    for (const auto &kv : updateColMap) {
        if (!first) sql += ", ";
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
    std::vector<std::string> vals;  // 存放实际字符串，防止被回收
    vals.reserve(paramCount);
    for (const auto &kv : updateColMap) {
        vals.push_back(kv.second);
        binds[idx].buffer_type = MYSQL_TYPE_STRING;
        binds[idx].buffer = (char*)vals.back().c_str();
        binds[idx].buffer_length = vals.back().size();
        idx++;
    }

    // 绑定 userID, deviceID
    vals.push_back(userID);
    binds[idx].buffer_type = MYSQL_TYPE_STRING;
    binds[idx].buffer = (char*)vals.back().c_str();
    binds[idx].buffer_length = vals.back().size();
    idx++;

    vals.push_back(deviceID);
    binds[idx].buffer_type = MYSQL_TYPE_STRING;
    binds[idx].buffer = (char*)vals.back().c_str();
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