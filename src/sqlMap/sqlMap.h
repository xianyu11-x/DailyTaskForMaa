#include <mysql/mysql.h>
#include <string>
#include <unordered_map>

//TODO:重构
struct MAAUser{
    std::string userID;
    std::string deviceID;
    std::string dailyTaskTime;
    std::string taskStartTime;
    std::string taskEndTime;
    std::string taskStrategy;
    std::string dailyTaskID;
};

int insertMAAUserInit(MYSQL *conn, std::string userID, std::string deviceID, std::string taskStrategy,std::string dailyTaskTime, std::string curTaskDefaultTime);

MAAUser queryMAAUserAllInfo(MYSQL *conn, std::string userID, std::string deviceID);

MAAUser queryMAAUserInfo(MYSQL *conn, std::string userID, std::string deviceID);

MAAUser queryMAAUserTaskStatus(MYSQL *conn, std::string userID, std::string deviceID);

MAAUser queryMAAUserStrategy(MYSQL *conn, std::string userID, std::string deviceID);

bool updateMAAUser(MYSQL *conn, std::string userID, std::string deviceID,std::unordered_map<std::string, std::string>& updateColMap);