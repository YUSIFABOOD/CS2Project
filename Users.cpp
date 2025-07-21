#include "include/Users.h"
#include "include/Authentication.h"
#include <fstream>
#include <stdexcept>

using namespace std;

// User Class Implementation
User::User() : hashedPass(""), salt("") { username = ""; }
User::User(const string& name, const string& hpass, const string& s)
    : hashedPass(hpass), salt(s) { username = name; }

string User::getUsername() const { return username; }

bool User::verifyPass(const string& pass) const {
    return hashedPass == Authentication::hashPass(pass, salt);
}

string User::getPass() const { return hashedPass; }

string User::getSalt() const { return salt; }

json User::toJson() const {
    return json{{"username", username}, {"hashedPass", hashedPass}, {"salt", salt}};
}

User User::fromJson(const json& j) {
    return User(j.at("username"), j.at("hashedPass"), j.at("salt"));
}

// UserStorage Class Implementation
void UserStorage::saveUsers(const unordered_map<string, User>& users, const string& filePath) {
    json usersJson = json::array();
    for (const auto& pair : users) {
        usersJson.push_back(pair.second.toJson());
    }

    ofstream file(filePath);
    if (!file.is_open()) {
        throw runtime_error("Failed to open file for writing: " + filePath);
    }
    file << usersJson.dump(4); // pretty print JSON
}

unordered_map<string, User> UserStorage::loadUsers(const string& filePath) {
    unordered_map<string, User> users;
    ifstream file(filePath);

    if (!file.is_open()) {
        // If the file doesn't exist, return an empty map. It will be created on first signup.
        return users;
    }
    json usersJson;
    // Check if the file is empty before parsing
    file.seekg(0, ios::end);
    if (file.tellg() == 0) {
        // File is empty
        return users;
    }
    file.seekg(0, ios::beg);
    try {
        file >> usersJson;
    } catch (const json::parse_error& e) {
        // If the file is corrupt, return an empty map.
        return users;
    }
    for (auto& u : usersJson)
    {
        string username = u["username"];
        string hashed = u["hashedPass"];
        string salt = u["salt"];
        users[username] = User(username, hashed, salt);
    }
    return users;
}