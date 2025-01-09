#include <mysql/mysql.h>
#include <string>
#include <unordered_map>
#include <vector>

//TODO:重构
struct MAAUser{
    std::string userID;
    std::string deviceID;
    std::string nextDailyTaskTime;
    std::string taskStartTime;
    std::string taskEndTime;
    std::string dailyTaskID;
};

struct MAADailyTaskPlan{
    std::string userID;
    std::string deviceID;
    std::string dailyTaskStrategy;
    std::string dailyTaskTime;
    int taskSeconds;
};

struct MAAQucikTask{
    std::string taskID;
    std::string userID;
    std::string deviceID;
    std::string taskCommitTime;
    std::string taskStartTime;
    std::string taskIsFinish;
    std::string taskActions;
};

struct MAAAction{
    std::string taskID;
    std::string actionID;
    std::string actionIsFinish;
};
    

int insertMAADailyTaskPlan(MYSQL *conn, const std::vector<MAADailyTaskPlan>& dailyTaskPlanList);

int insertMAAQucikTask(MYSQL *conn, const std::vector<MAAQucikTask>& quickTaskList);

int insertMAAAction(MYSQL *conn, const std::vector<MAAAction>& actionList);

int insertMAAUserInit(MYSQL *conn, std::string userID, std::string deviceID, std::string taskStrategy,std::string dailyTaskTime, std::string curTaskDefaultTime);

MAAUser queryMAAUserAllInfo(MYSQL *conn, std::string userID, std::string deviceID);

MAAUser queryMAAUserInfo(MYSQL *conn, std::string userID, std::string deviceID);

MAAUser queryMAAUserTaskStatus(MYSQL *conn, std::string userID, std::string deviceID);

MAAUser queryMAAUserStrategy(MYSQL *conn, std::string userID, std::string deviceID);

bool updateMAAUser(MYSQL *conn, std::string userID, std::string deviceID,std::unordered_map<std::string, std::string>& updateColMap);