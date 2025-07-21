#include "include/Notification.h"
#include <iomanip>
#include <sstream>

Notification::Notification(const std::string & messagee): message(messagee), timestamp(std::time(nullptr)){}

std::string Notification::getMessage() const {
    return message;
}

std::string Notification::getFormattedTime() const {
    char timing[80];
    std::tm*timeInfo =std::localtime(&timestamp);
    std::strftime(timing, sizeof(timing), "%Y-%m-%d %H:%M:%S", timeInfo);
    return std::string(timing);
}


