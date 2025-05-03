#include <mysql/mysql.h>
#include <string>
#include <unordered_map>
#include <vector>
#pragma once
//TODO:重构
struct MAAUser{
    std::string userID;
    std::string deviceID;
    std::string nextDailyTaskTime;
    std::string dailyTaskStartTime;
    std::string dailyTaskEndTime;
    std::string dailyTaskID;
};

struct MAADailyTaskPlan{
    std::string planID;
    std::string userID;
    std::string deviceID;
    std::string dailyTaskStrategy;
    std::string dailyTaskTime;
    int taskSeconds;
};

struct MAAQuickTask{
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

int insertMAAQuickTask(MYSQL *conn, const std::vector<MAAQuickTask>& quickTaskList);

int insertMAAAction(MYSQL *conn, const std::vector<MAAAction>& actionList);

int insertMAAUser(MYSQL *conn, const std::vector<MAAUser>& userList);

std::vector<MAAUser> queryMAAUserAllInfo(MYSQL *conn, std::string userID, std::string deviceID);

std::vector<MAAUser> queryMAAUserInfo(MYSQL *conn, std::string userID, std::string deviceID);

std::vector<MAAUser> queryMAAUserTaskStatus(MYSQL *conn, std::string userID, std::string deviceID);

std::vector<MAADailyTaskPlan> queryMAAUserStrategy(MYSQL *conn, std::string userID, std::string deviceID);

std::vector<MAAQuickTask> queryMAAQuickTask(MYSQL *conn, std::string userID, std::string deviceID,std::string taskIsFinish);

std::vector<MAAAction> queryMAAAction(MYSQL *conn, std::string actionID);

std::vector<MAAAction> queryMAAAction(MYSQL *conn, std::string taskID, std::string actionIsFinish);



bool updateMAAUser(MYSQL *conn, std::string userID, std::string deviceID,std::unordered_map<std::string, std::string>& updateColMap);

bool updateMAAAction(MYSQL *conn,std::string actionID,std::string actionIsFinish);

bool updateMAAQuickTask(MYSQL *conn, std::string taskID,std::unordered_map<std::string, std::string>& updateColMap);

bool updateMAADailyTaskPlan(MYSQL *conn, std::string planID,std::string userID,std::string deviceID,std::unordered_map<std::string, std::string>& updateColMap);

bool deleteMAADailyTaskPlan(MYSQL *conn, std::string planID);

bool deleteAllMAADailyTaskPlan(MYSQL *conn, std::string userID, std::string deviceID);