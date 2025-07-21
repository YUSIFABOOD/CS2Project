#ifndef USERS_H
#define USERS_H

#include <nlohmann/json.hpp>

// Forward declaration if needed, or just for clarity
using json = nlohmann::json;
#include <bits/stdc++.h>
using namespace std;
class BaseUser
{
    protected:
    string username;
    public:
    virtual string getUsername() const=0;
};

class User: public BaseUser
{
    // vector<Post> posts;
    string hashedPass;
    string salt;
    public:
    User();
    User(const string& name, const string& hpass, const string& s); 
    bool verifyPass(const string& pass) const;
    // void addPost(string content) const override{};
    // void displayProfile() const override{};
    // void addReview(string content, float rating) {};
    string getUsername() const override;
    string getPass() const;
    string getSalt() const;

    // JSON serialization
    json toJson() const;
    static User fromJson(const json& j);
};

// class Guest: public BaseUser
// {
//     public:
//     Guest() {username="Guest";}
//     string getUsername() const override{}
// };

class UserStorage 
{
public:
    static void saveUsers(const unordered_map<string, User>& users, const string& filePath);
    static unordered_map<string, User> loadUsers(const string& filePath);
};
#endif