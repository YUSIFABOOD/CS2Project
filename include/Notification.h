#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <string>
#include <ctime>

class Notification {
private:
    std::string message;
    std::time_t timestamp;

public:
    Notification(const std::string&);
    std::string getMessage() const;
    std::string getFormattedTime() const;
};


#endif
