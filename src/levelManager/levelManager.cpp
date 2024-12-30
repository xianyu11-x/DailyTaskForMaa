#include "levelManager.h"
#include "../utils/regUtil.hpp"
#include <string>

levelManager *levelManager::GetInstance()
{
    static levelManager instance;
    return &instance;
}

void levelManager::setPermanentLevel(std::string PermanentLevelJson)
{
    permanentLevelDict.clear();
    rapidjson::Document doc;
    doc.Parse(PermanentLevelJson.c_str());
    const rapidjson::Value& levels = doc["permanentLevel"];
    for (const auto& level : levels.GetArray())
    {
        std::string levelName = level.MemberBegin()->name.GetString();
        std::vector<int> levelStatus;
        for (auto& v : level.MemberBegin()->value.GetArray())
        {
            levelStatus.push_back(v.GetInt());
        }
        permanentLevelDict[levelName] = levelStatus;
    }
}

void levelManager::setSideStoryLevel(const rapidjson::Value::Array& sideStoryLevelList,const std::string& startTime,const std::string& endTime)
{
    sideStoryLevelDict.clear();
    for (const auto& level : sideStoryLevelList)
    {
        std::string levelName = level["Value"].GetString();
        sideStoryLevelDict[levelName] = {1,1,1,1,1,1,1,};
    }
    sideStoryStartTime = startTime;
    sideStoryEndTime = endTime;
}

std::pair<std::string, std::string> levelManager::getSideStoryTime(){
    return std::make_pair(sideStoryStartTime,sideStoryEndTime);
}

std::string levelManager::getDefaultSideStoryLevel(){
    int maxLevelNum = 0;
    std::string curlevelPrefix = "";
    for(const auto& [k,v]:sideStoryLevelDict){
        if(isLevelName(k)){
            auto [levelPrefix,levelNum] = getLevelPara(k);
            if(curlevelPrefix == ""){
                curlevelPrefix = levelPrefix;
            }
            if(levelNum > maxLevelNum){
                maxLevelNum = levelNum;
            }
        }
    }
    if(sideStoryLevelDict.count(curlevelPrefix + "-" + std::to_string(maxLevelNum-1))){
        return curlevelPrefix + "-" + std::to_string(maxLevelNum-1);
    }else{
        return curlevelPrefix + "-" + std::to_string(maxLevelNum);
    }
}

bool levelManager::checkLevelStatus(std::string levelName, int dayOfWeek){
    if(sideStoryLevelDict.count(levelName)){
        return true;
    }
    if(permanentLevelDict.count(levelName)){
        return permanentLevelDict[levelName][dayOfWeek];
    }
    return false;
}