
#include "../utils/jsonUtil.hpp"
#include "levelManager.h"
#include "rapidjson/allocators.h"
#include "rapidjson/document.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

rapidjson::Value setLevelObj(const std::string &levelName,
                             std::vector<int> levelStatus,
                             rapidjson::MemoryPoolAllocator<> &allocator) {
  rapidjson::Value levelObj(rapidjson::kObjectType);
  rapidjson::Value levelStatusArray(rapidjson::kArrayType);
  for (auto status : levelStatus) {
    rapidjson::Value statusValue(status);
    levelStatusArray.PushBack(statusValue, allocator);
  }
  rapidjson::Value levelNameValue;
  levelNameValue.SetString(levelName.c_str(), allocator);
  levelObj.AddMember(levelNameValue, levelStatusArray, allocator);
  return levelObj;
}

int main() {
  rapidjson::Document doc;
  doc.SetObject();
  auto &allocator = doc.GetAllocator();
  rapidjson::Value levelArray(rapidjson::kArrayType);
  levelArray.PushBack(setLevelObj("CE-6", {1, 0, 1, 0, 1, 0, 1}, allocator),
                      allocator);
  levelArray.PushBack(setLevelObj("AP-5", {1, 1, 0, 0, 1, 0, 1}, allocator),
                      allocator);
  levelArray.PushBack(setLevelObj("LS-6", {1, 1, 1, 1, 1, 1, 1}, allocator),
                      allocator);
  levelArray.PushBack(setLevelObj("CA-5", {1, 0, 1, 1, 0, 1, 0}, allocator),
                      allocator);
  levelArray.PushBack(setLevelObj("SK-5", {0, 1, 0, 1, 0, 1, 1}, allocator),
                      allocator);

  levelArray.PushBack(setLevelObj("PR-A-1", {1, 1, 0, 0, 1, 1, 0}, allocator),
                      allocator);
  levelArray.PushBack(setLevelObj("PR-A-2", {1, 1, 0, 0, 1, 1, 0}, allocator),
                      allocator);
  levelArray.PushBack(setLevelObj("PR-B-1", {0, 1, 1, 0, 0, 1, 1}, allocator),
                      allocator);
  levelArray.PushBack(setLevelObj("PR-B-2", {0, 1, 1, 0, 0, 1, 1}, allocator),
                      allocator);
  levelArray.PushBack(setLevelObj("PR-C-1", {1, 0, 0, 1, 1, 0, 1}, allocator),
                      allocator);
  levelArray.PushBack(setLevelObj("PR-C-2", {1, 0, 0, 1, 1, 0, 1}, allocator),
                      allocator);
  levelArray.PushBack(setLevelObj("PR-D-1", {1, 0, 1, 1, 0, 0, 1}, allocator),
                      allocator);
  levelArray.PushBack(setLevelObj("PR-D-2", {1, 0, 1, 1, 0, 0, 1}, allocator),
                      allocator);

  levelArray.PushBack(setLevelObj("1-7", {1, 1, 1, 1, 1, 1, 1}, allocator),
                      allocator);
  levelArray.PushBack(setLevelObj("剿灭模式", {1, 1, 1, 1, 1, 1, 1}, allocator),
                      allocator);
  doc.AddMember("permanentLevel", levelArray, allocator);
  std::string jsonString = jsonToString(doc);
  std::cout << jsonString << std::endl;

  // 将jsonString写入文件
  std::ofstream ofs("permanentLevel.json");
  ofs << jsonString;
  ofs.close();

  auto levelManager = levelManager::GetInstance();
  levelManager->setPermanentLevel(jsonString);

  // 读取json文件
  // std::ifstream ifs("StageActivity.json");
  // std::stringstream strbuffer;
  // strbuffer << ifs.rdbuf();
  // std::string str = strbuffer.str();
  // rapidjson::Document requestDOM;
  // requestDOM.Parse(str.c_str());
  // auto sideStoryLevelList = requestDOM["Official"]["sideStoryStage"].GetArray();
  // if (!sideStoryLevelList.Empty()) {
  //   auto startTime =
  //       sideStoryLevelList[0]["Activity"]["UtcStartTime"].GetString();
  //   auto endTime =
  //       sideStoryLevelList[0]["Activity"]["UtcExpireTime"].GetString();
  //   std::cout << startTime << std::endl;
  //   std::cout << endTime << std::endl;
  //   levelManager->setSideStoryLevel(sideStoryLevelList, startTime, endTime);
  // }
  // std::cout << levelManager->getDefaultSideStoryLevel() << std::endl;
  return 0;
}