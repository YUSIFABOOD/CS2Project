#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H
#include <bits/stdc++.h>
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
    public:
    Authentication();
    
    string login (string name, string pass);
    void signup (string name, string pass);
    void logout(const string& token);

    User* getUserByToken (const string& token);
    bool isLoggedIn(const string& token)const;

    static string hashPass (const string& pass, const string& salt);
    static string generateSalt ();
    static string generateSession();
    bool userExists(const string& name) const;
};
#endif