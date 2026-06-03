#include "levelManager/levelManager.h"
#include "utils/timeUtil.hpp"
#include "co_async/awaiter/task.hpp"
#include "co_async/utils/expected.hpp"
#include "rapidjson/document.h"
#include <ctime>
#include <mysql.h>
#include <string>
#include <vector>
#include "updateLevel.h"

namespace {

bool isActiveActivity(const rapidjson::Value& activity, const std::tm& tmNow)
{
    if (!activity.IsObject() || !activity.HasMember("UtcStartTime")
        || !activity["UtcStartTime"].IsString()
        || !activity.HasMember("UtcExpireTime")
        || !activity["UtcExpireTime"].IsString()) {
        return false;
    }
    return isTimeAfter(tmNow,
               stringToTm(activity["UtcStartTime"].GetString(), "%Y/%m/%d %H:%M:%S"))
        && isTimeAfter(
            stringToTm(activity["UtcExpireTime"].GetString(), "%Y/%m/%d %H:%M:%S"),
            tmNow);
}

void appendActiveStages(const rapidjson::Value& activityInfo,
    const std::tm& tmNow, std::vector<std::string>& levelList,
    std::string& startTime, std::string& endTime, bool& hasAvailableTime)
{
    if (!activityInfo.IsObject() || !activityInfo.HasMember("Activity")
        || !isActiveActivity(activityInfo["Activity"], tmNow)) {
        return;
    }

    const auto& activity = activityInfo["Activity"];
    if (!hasAvailableTime) {
        startTime = activity["UtcStartTime"].GetString();
        endTime = activity["UtcExpireTime"].GetString();
        hasAvailableTime = true;
    }

    if (activityInfo.HasMember("Stages") && activityInfo["Stages"].IsArray()) {
        for (const auto& stage : activityInfo["Stages"].GetArray()) {
            if (stage.IsObject() && stage.HasMember("Value")
                && stage["Value"].IsString()) {
                levelList.push_back(stage["Value"].GetString());
            }
        }
    } else if (activityInfo.HasMember("Value") && activityInfo["Value"].IsString()) {
        levelList.push_back(activityInfo["Value"].GetString());
    }
}

} // namespace

Task<Expected<>> updateLevel(HTTPServer::IO& io)
{
    auto request = co_await io.request_body();
    rapidjson::Document requestDOM;
    requestDOM.Parse(request.value().c_str());
    if (requestDOM.HasParseError() || !requestDOM.IsObject()
        || !requestDOM.HasMember("Official")
        || !requestDOM["Official"].IsObject()
        || !requestDOM["Official"].HasMember("sideStoryStage")) {
        co_await co_await HTTPServerUtils::make_ok_response(io,
            "invalid sideStory json");
        co_return {};
    }

    auto levelManager = levelManager::GetInstance();
    const auto& sideStoryStage = requestDOM["Official"]["sideStoryStage"];
    std::vector<std::string> levelList;
    std::string startTime;
    std::string endTime;
    bool hasAvailableTime = false;
    std::tm tmNow = getNowTm();

    if (sideStoryStage.IsArray()) {
        for (const auto& level : sideStoryStage.GetArray()) {
            appendActiveStages(level, tmNow, levelList, startTime, endTime,
                hasAvailableTime);
        }
    } else if (sideStoryStage.IsObject()) {
        for (auto iter = sideStoryStage.MemberBegin();
             iter != sideStoryStage.MemberEnd(); ++iter) {
            appendActiveStages(iter->value, tmNow, levelList, startTime,
                endTime, hasAvailableTime);
        }
    }

    if (!levelList.empty())
        levelManager->setSideStoryLevel(levelList, startTime, endTime);
    co_await co_await HTTPServerUtils::make_ok_response(io,
        "updateSideStoryLevel");
    co_return {};
}
