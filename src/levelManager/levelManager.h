#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#pragma once
class levelManager {
    public:
        static levelManager *GetInstance();

        void setPermanentLevel(std::string PermanentLevelJson);

        void setSideStoryLevel(const rapidjson::Value::Array& sideStoryLevelList,const std::string& startTime,const std::string& endTime);

        std::string getDefaultSideStoryLevel();

        bool checkLevelStatus(std::string levelName, int dayOfWeek);

        std::pair<std::string, std::string> getSideStoryTime();
    private:
        levelManager() = default;
        levelManager(const levelManager&) = delete;
        levelManager& operator=(const levelManager&) = delete;
    
        std::unordered_map<std::string, std::vector<int>> permanentLevelDict;
        std::unordered_map<std::string, std::vector<int>> sideStoryLevelDict;
        std::string sideStoryStartTime;
        std::string sideStoryEndTime;

};