#include <bits/stdc++.h>
#include <openssl/evp.h>
#include <nlohmann/json.hpp>
#include "include/Authentication.h"
#include "include/Users.h"
using namespace std;
using json = nlohmann::json;

Authentication::Authentication(const string& db_path) : db_path_(db_path) {
    // Set sessions path to be in the same directory as users database
    size_t lastSlash = db_path.find_last_of("/\\");
    string dir = (lastSlash != string::npos) ? db_path.substr(0, lastSlash + 1) : "";
    sessions_path_ = dir + "sessions.json";
    
    try {
        cout << "Loading users from: " << db_path << endl;
        usersByUsername = UserStorage::loadUsers(db_path_);
        loadSessions();
    } catch (const exception& e) {
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
    saveSessions();
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
        usersByUsername.emplace(name, User(name, hpass, salt));
        UserStorage::saveUsers(usersByUsername, db_path_);
    } catch (const exception& e) {
        throw runtime_error("Failed to create user: " + string(e.what()));
    }
}

void Authentication::logout(const string& token)
        {   if(sessions.count(token)){
            usersnameToToken.erase(sessions[token]);
            sessions.erase(token);
            saveSessions();
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
            unsigned char hash[EVP_MAX_MD_SIZE];
            unsigned int hash_len;
            
            EVP_MD_CTX* ctx = EVP_MD_CTX_new();
            if (!ctx) {
                throw runtime_error("Failed to create hash context");
            }
            
            if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1 ||
                EVP_DigestUpdate(ctx, input.c_str(), input.length()) != 1 ||
                EVP_DigestFinal_ex(ctx, hash, &hash_len) != 1) {
                EVP_MD_CTX_free(ctx);
                throw runtime_error("Failed to compute hash");
            }
            
            EVP_MD_CTX_free(ctx);

            stringstream ss;
            for (unsigned int i = 0; i < hash_len; i++) {
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

string Authentication::verifyToken(const string& token) {
    if (sessions.find(token) == sessions.end()) {
        throw runtime_error("Invalid or expired token");
    }
    return sessions[token];
}

void Authentication::loadSessions() {
    ifstream file(sessions_path_);
    if (!file.is_open()) {
        cout << "No existing sessions file found, starting with empty sessions" << endl;
        return; // File might not exist on first run
    }
    
    json data;
    // Check if the file is empty before parsing
    file.seekg(0, ios::end);
    if (file.tellg() == 0) {
        cout << "Sessions file is empty" << endl;
        return;
    }
    file.seekg(0, ios::beg);
    
    try {
        file >> data;
        if (data.contains("sessions")) {
            sessions.clear();
            usersnameToToken.clear();
            for (const auto& session : data["sessions"]) {
                string token = session["token"];
                string username = session["username"];
                sessions[token] = username;
                usersnameToToken[username] = token;
            }
            cout << "Loaded " << sessions.size() << " sessions" << endl;
        }
    } catch (const exception& e) {
        cout << "Error loading sessions: " << e.what() << endl;
    }
}

void Authentication::saveSessions() {
    json sessions_json = json::array();
    for (const auto& session : sessions) {
        json session_obj;
        session_obj["token"] = session.first;
        session_obj["username"] = session.second;
        sessions_json.push_back(session_obj);
    }
    
    json final_json;
    final_json["sessions"] = sessions_json;
    
    ofstream file(sessions_path_);
    if (file.is_open()) {
        file << final_json.dump(4);
        cout << "Saved " << sessions.size() << " sessions" << endl;
    } else {
        cerr << "Error: Could not save sessions to " << sessions_path_ << endl;
    }
}

