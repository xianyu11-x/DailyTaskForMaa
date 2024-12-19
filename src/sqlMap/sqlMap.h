#include <mysql/mysql.h>
#include <string>

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

MAAUser quertMAAUserTaskStatus(MYSQL *conn, std::string userID, std::string deviceID);
