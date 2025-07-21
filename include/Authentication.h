#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H
#include <bits/stdc++.h>
#include <nlohmann/json.hpp>
#include "Users.h"
using namespace std;
class Authentication 
{
    //maps usernames to their users
    unordered_map<string, User> usersByUsername;
    //maps tokens to usernames
    unordered_map<string, string> sessions;
    //maps users to tokens
    unordered_map<string, string> usersnameToToken;
    string db_path_;
    string sessions_path_;
    public:
    Authentication(const string& db_path);
    
    string login (string name, string pass);
    void signup (string name, string pass);
    void logout(const string& token);

    User* getUserByToken (const string& token);
    bool isLoggedIn(const string& token)const;
    string verifyToken(const string& token); // Returns username if token is valid, throws exception if not

    static string hashPass (const string& pass, const string& salt);
    static string generateSalt ();
    static string generateSession();
    bool userExists(const string& name) const;
    
    // Session persistence methods
    void loadSessions();
    void saveSessions();
};
#endif