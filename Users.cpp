#include <bits/stdc++.h>
#include "include/Users.h"
#include "include/Authentication.h"
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

User::User(): hashedPass(""),salt(""){username="";};
User::User(string name, string pass, string s): hashedPass(pass), salt(s){username=name;}; 
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
const std::vector<Notification>& User::getNotification() const
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
