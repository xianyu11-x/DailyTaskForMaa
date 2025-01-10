

#include <regex>
#include <string>
#include <utility>
#pragma once
bool isLevelName(const std::string &name) {
    const std::regex pattern(R"(^[A-Za-z0-9]+-[0-9]+$)");
    return std::regex_match(name, pattern);
}

std::pair<std::string, int> getLevelPara(const std::string &name) {
    const std::regex pattern(R"(^([A-Za-z0-9]+)-([0-9]+)$)");
    std::smatch match;
    if (std::regex_match(name, match, pattern)) {
        // match[1] 就是数字部分
        std::string levelStr = match[1].str();
        std::string numberStr = match[2].str();
        int number = std::stoi(numberStr);
        return {levelStr, number};
    }
    return {"", -1};
}