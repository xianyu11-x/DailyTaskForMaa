#include "levelManager/levelManager.h"
#include "utils/timeUtil.hpp"
#include "co_async/awaiter/task.hpp"
#include "co_async/utils/expected.hpp"
#include "rapidjson/document.h"
#include <ctime>
#include <mysql/mysql.h>
#include <string>
#include <vector>
#include "updateLevel.h"

Task<Expected<>> updateLevel(HTTPServer::IO& io)
{
    auto request = co_await io.request_body();
    rapidjson::Document requestDOM;
    requestDOM.Parse(request.value().c_str());
    auto levelManager = levelManager::GetInstance();
    auto sideStoryLevelList = requestDOM["Official"]["sideStoryStage"].GetArray();
    std::vector<std::string> levelList;
    std::string startTime;
    std::string endTime;
    bool hasAvailableTime = false;
    std::tm tmNow = getNowTm();
    for (const auto& level : sideStoryLevelList) {
        auto tmpStartTime = level["Activity"]["UtcStartTime"].GetString();
        auto tmpEndTime = level["Activity"]["UtcExpireTime"].GetString();
        auto levelName = level["Value"].GetString();
        if (isTimeAfter(tmNow, stringToTm(tmpStartTime, "%Y/%m/%d %H:%M:%S")) && isTimeAfter(stringToTm(tmpEndTime, "%Y/%m/%d %H:%M:%S"), tmNow)) {
            levelList.push_back(levelName);
            if (!hasAvailableTime) {
                startTime = tmpStartTime;
                endTime = tmpEndTime;
                hasAvailableTime = true;
            }
        }
    }
    if (!levelList.empty())
        levelManager->setSideStoryLevel(levelList, startTime, endTime);
    co_await co_await HTTPServerUtils::make_ok_response(io,
        "updateSideStoryLevel");
    co_return {};
}