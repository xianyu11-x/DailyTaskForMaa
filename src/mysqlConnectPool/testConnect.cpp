#include "co_async/awaiter/task.hpp"
#include "co_async/utils/expected.hpp"
#include "sqlConnectPool.h"
#include <co_async/co_async.hpp>
#include <mysql/field_types.h>
#include <string>

using namespace co_async;

Task<Expected<>> amain()
{
    connectionPool *connPool = connectionPool::GetInstance();
    connPool->init("172.23.17.52","root","root","MAABackendDB",3306,10);
    auto conn = co_await connPool->GetConnection();
    std::string jsonString = "{\"userID\":\"123456\",\"deviceID\":\"abcdefg\"}";
    std::string userID = "abc";
    std::string deviceID = "def";
    const char* query = "INSERT INTO MAAUser (UserID, DeviceID, taskStrategy) VALUES (?, ?, ?) ON DUPLICATE KEY UPDATE taskStrategy = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt){
        co_await co_await stdio().putline("init sql error"s);
        co_await connPool->ReleaseConnection(conn);
        co_return {};
    }
    if(mysql_stmt_prepare(stmt, query, strlen(query))){
        co_await co_await stdio().putline("prepare sql error"s);
        co_await connPool->ReleaseConnection(conn);
        co_return {};
    }
    MYSQL_BIND bind[4];
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)userID.c_str();
    bind[0].buffer_length = userID.length();

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (char*)deviceID.c_str();
    bind[1].buffer_length = deviceID.length();

    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (char*)jsonString.c_str();
    bind[2].buffer_length = jsonString.length();

    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = (char*)jsonString.c_str();
    bind[3].buffer_length = jsonString.length();

    if(mysql_stmt_bind_param(stmt, bind)){
        co_await co_await stdio().putline("bind sql error"s);
        co_await connPool->ReleaseConnection(conn);
        co_return {};
    }

    if(mysql_stmt_execute(stmt)){
        co_await co_await stdio().putline("execute sql error"s);
        co_await connPool->ReleaseConnection(conn);
        co_return {};
    }else{
        co_await co_await stdio().putline("execute sql success"s);
    }

    co_await connPool->ReleaseConnection(conn);
    co_return {};
}   

int main()
{
    co_main(amain());
    return 0;
}