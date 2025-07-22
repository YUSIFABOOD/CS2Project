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

void Notification::setMessage(const std::string& messagee) { message = messagee; }

void Notification::setTimestamp(std::time_t ts)
{timestamp=ts;}
std::time_t Notification::getTimestamp() const {return timestamp;}

void to_json(json& j, const Notification& n)
{
    j = json{
            {"message", n.getMessage()},
            {"timestamp", n.getTimestamp()}
        };
}

void from_json(const json& j, Notification& n)
{
    n.setMessage(j.at("message").get<std::string>());
    n.setTimestamp(j.at("timestamp").get<std::time_t>());
}