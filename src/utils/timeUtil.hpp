#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

std::tm stringToTm(const std::string& timeStr, const std::string& format) {
    std::tm tm = {};
    std::istringstream ss(timeStr);
    ss >> std::get_time(&tm, format.c_str());
    if (ss.fail()) {
        throw std::runtime_error("Failed to parse time string");
    }
    return tm;
}

bool isTimeAfter(const std::tm& lhs, const std::tm& rhs) {
    return std::mktime(const_cast<std::tm*>(&lhs)) > std::mktime(const_cast<std::tm*>(&rhs));
}

std::tm DateAdd(const std::tm& oriTm, const std::tm& addTm) {
    std::time_t oriTime = std::mktime(const_cast<std::tm*>(&oriTm));
    std::time_t addTime = std::mktime(const_cast<std::tm*>(&addTm));
    std::time_t newTime = oriTime + addTime;
    std::tm newTm = *std::localtime(&newTime);
    return newTm;
}