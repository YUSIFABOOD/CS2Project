#include <bits/stdc++.h>
#include <openssl/sha.h>
#include "include/Authentication.h"
#include "include/Users.h"
using namespace std;

Authentication::Authentication() {
    try {
        // Use relative path from build directory
        const string db_path = "../database/users.csv";
        cout << "Loading users from: " << db_path << endl;
        usersByUsername = UserStorage::loadUsers(db_path);
    } catch (const std::exception& e) {
        throw runtime_error("Failed to initialize authentication system: " + string(e.what()));
    }
}

string Authentication::login(string name, string pass) {
    if (name.empty() || pass.empty()) {
        throw runtime_error("Username and password cannot be empty");
    }

    if (!userExists(name)) {
        throw runtime_error("User not found");
    }

    User& user = usersByUsername[name];
    if (!user.verifyPass(pass)) {
        throw runtime_error("Invalid password");
    }

    if (usersnameToToken.count(name)) {
        return usersnameToToken[name]; // Return existing token
    }

    string token = generateSession();
    sessions[token] = name;
    usersnameToToken[name] = token;
    return token;
}

void Authentication::signup(string name, string pass) {
    if (name.empty() || pass.empty()) {
        throw runtime_error("Username and password cannot be empty");
    }
    
    if (name.length() < 3) {
        throw runtime_error("Username must be at least 3 characters long");
    }

    if (pass.length() < 6) {
        throw runtime_error("Password must be at least 6 characters long");
    }

    if (userExists(name)) {
        throw runtime_error("User already exists");
    }

    try {
        string salt = generateSalt();
        string hpass = hashPass(pass, salt);
        usersByUsername.insert({name, User(name, hpass, salt)});
        const string db_path = "../database/users.csv";
        UserStorage::saveUser(usersByUsername[name], db_path);
    } catch (const exception& e) {
        throw runtime_error("Failed to create user: " + string(e.what()));
    }
}

void Authentication::logout(const string& token)
        {   if(sessions.count(token)){
            usersnameToToken.erase(sessions[token]);
            sessions.erase(token);
            }
        }


User* Authentication::getUserByToken (const string& token)
        {
            if(!isLoggedIn(token)) return nullptr;
            else 
                {string user=sessions[token];
                return &usersByUsername[user];}
        }


bool Authentication::isLoggedIn(const string& token) const
        {
            return (sessions.count(token)>0);
        }


string Authentication::hashPass (const string& pass, const string& salt) {
            string input = pass + salt;
            unsigned char hash[SHA256_DIGEST_LENGTH];
            SHA256_CTX sha256;
            SHA256_Init(&sha256);
            SHA256_Update(&sha256, input.c_str(), input.length());
            SHA256_Final(hash, &sha256);

            stringstream ss;
            for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
                ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
            }
            return ss.str();
        }


string Authentication::generateSalt ()
        {const int length = 16;
            const char charset[] =
                "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";
            string salt;
            for (int i = 0; i < length; i++) {
                salt += charset[rand() % (sizeof(charset) - 1)];
            }
            return salt;}


string Authentication::generateSession(){
            const int length = 32;
            const char charset[] =
                "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";
            string token;
            for (int i = 0; i < length; i++) {
                token += charset[rand() % (sizeof(charset) - 1)];
            }
            return token;
        }

bool Authentication::userExists(const string& name) const {return usersByUsername.count(name);}

