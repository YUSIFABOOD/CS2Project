#ifndef USERS_H
#define USERS_H
#include <bits/stdc++.h>
#include <nlohmann/json.hpp>
#include "AVLTree.h"

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
    AVLTree<string> friends;
    public:
    User();
    User(const string& name, const string& pass, const string& s); 
    bool verifyPass(const string& pass) const;
    // void addPost(string content) const override{};
    // void displayProfile() const override{};
    // void addReview(string content, float rating) {};
    string getUsername() const override;
    string getPass() const;
    string getSalt() const;
    const AVLTree<string>& getFriendTree() const;
    AVLTree<string>& getFriendTree();
    
    // JSON serialization methods
    nlohmann::json toJson() const;
    static User fromJson(const nlohmann::json& j);

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
    static void saveUsers(const unordered_map<string, User>& users, const string& filename);
    static unordered_map<string, User> loadUsers(const string& filename);
};
#endif