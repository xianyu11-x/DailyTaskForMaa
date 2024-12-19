#include "sqlMap.h"
#include <cstring>
#include <string>


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