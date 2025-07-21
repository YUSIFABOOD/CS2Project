#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <string>
#include <ctime>
#include "json.hpp"

using json = nlohmann::json;

class Notification {
private:
    std::string message;
    std::time_t timestamp;

public:
    Notification() = default;
    Notification(const std::string&);
    std::string getMessage() const;
    std::string getFormattedTime() const;
    void setMessage(const std::string&);
    std::time_t getTimestamp() const;
    void setTimestamp(std::time_t ts);
};



void to_json(json& j, const Notification& n);
void from_json(const json& j, Notification& n);

#endif
