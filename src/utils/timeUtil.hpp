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

std::string tmToString(const std::tm& tm, const std::string& format) {
    std::ostringstream oss;
    oss << std::put_time(&tm, format.c_str());
    return oss.str();
}


bool isTimeAfter(const std::tm& lhs, const std::tm& rhs) {
    return std::mktime(const_cast<std::tm*>(&lhs)) > std::mktime(const_cast<std::tm*>(&rhs));
}

std::tm DateAdd(const std::tm& oriTm, int seconds) {
    std::time_t oriTime = std::mktime(const_cast<std::tm*>(&oriTm));
    std::time_t newTime = oriTime + seconds;
    std::tm newTm = *std::localtime(&newTime);
    return newTm;
}