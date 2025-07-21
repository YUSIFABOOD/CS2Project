#ifndef USERS_H
#define USERS_H
#include <bits/stdc++.h>
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
    User(string name, string pass, string s); 
    bool verifyPass(string& pass) const;
    // void addPost(string content) const override{};
    // void displayProfile() const override{};
    // void addReview(string content, float rating) {};
    string getUsername() const override;
    string getPass() const;
    string getSalt() const;
    const AVLTree<string>& getFriendTree() const;

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
    static void saveUser(const User& user, const string& filename);
    static unordered_map<string, User> loadUsers(const string& filename);
};
#endif