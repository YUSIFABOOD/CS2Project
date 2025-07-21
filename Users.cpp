#include <bits/stdc++.h>
#include "include/Users.h"
#include "include/Authentication.h"
#include "include/Notification.h"
#include "json.hpp"
#include <vector>
#include <string>
#include <filesystem>
using json = nlohmann::json;
using namespace std;
namespace fs = std::filesystem;

User::User(): hashedPass(""),salt(""){username="";};
User::User(string name, string pass, string s): hashedPass(pass), salt(s){username=name;}
User::~User() {}

bool User::verifyPass(string& pass) const
{
    return (hashedPass==Authentication::hashPass(pass, salt));
};

string User::getUsername() const {return username;}
string User::getPass() const {return hashedPass;}
string User::getSalt() const {return salt;}

void User::addNotification(const std::string & messaaggee)
{
    notifications.emplace_back(messaaggee);
}
const std::vector<Notification>& User::getNotifications() const
{
    return notifications;
}
void User::clearNotification(int index)
{
    if(index>=0 && index<notifications.size())
        notifications.erase(notifications.begin()+index);
}
void User::clearAllNotifications()
{
        notifications.clear();
}

void User::saveNotificationsToFile(const std::string& filename) const {
    try{
        json j = notifications;
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file for saving notifications: " << filename << std::endl;
            return;
        }
        file << j.dump(4);
        if (file.fail()) {
            std::cerr << "Error: Failed to write notifications to file: " << filename << std::endl;
        }
    }
    catch (const std::exception& e) {
    std::cerr << "Exception while saving notifications: " << e.what() << std::endl;
        }
}


void User::loadNotificationsFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open notification file: " << filename << std::endl;
        return;
    }
    if (file.peek() == std::ifstream::traits_type::eof()) {
           notifications.clear();
           return;
       }

       try {
           json j;
           file >> j;
           notifications = j.get<std::vector<Notification>>();
       } catch (const std::exception& e) {
           std::cerr << "Warning: Failed to parse notifications JSON: " << e.what() << std::endl;
       }
   }
}


void UserStorage::saveUser(const User& user, const string& filename)
{
   
    // Try to open file in append mode
    ofstream file(filename, ios::app);
    if(!file.is_open()) {
        throw runtime_error("Failed to open file for writing: " + filename);
    }

    // Write user data
    file << user.getUsername() << ','
         << user.getPass() << ','
         << user.getSalt() << '\n';
    
    file.close();
    
    // Verify the write was successful
    if (file.fail()) {
        throw runtime_error("Failed to write to file: " + filename);
    }
}

unordered_map<string, User> UserStorage::loadUsers(const string& filename)
{
    unordered_map<string, User> users;
    
    // Try to open and read the file
    ifstream file(filename);
    if(!file.is_open()) {
        throw runtime_error("Failed to open file for reading: " + filename);
    }

    string line;
    while(getline(file, line)) {
        if(line.empty()) continue;  // Skip empty lines
        
        stringstream ss(line);
        string name, pass, salt;
        
        if(getline(ss, name, ',') && 
           getline(ss, pass, ',') && 
           getline(ss, salt, ',')) {
            users[name] = User(name, pass, salt);
        }
    }
    
    return users;
}
