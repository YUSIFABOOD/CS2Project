#include "include/FriendsManager.h"
#include "include/AVLTree.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <nlohmann/json.hpp>
using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;

// Constructor: initializes with reference to existing user map
FriendsManager::FriendsManager(unordered_map<string, User>& usersMap)
    : users(usersMap) {}

// Send a friend request from -> to
bool FriendsManager::sendFriendRequest(const string& from, const string& to) {
    // Validate users exist
    if (users.find(from) == users.end() || users.find(to) == users.end()) {
        throw runtime_error("User not found");
    }

    // Already friends?
    if (areFriends(from, to)) {
        cout << "Users are already friends" << endl;
        return false;
    }

    // Already requested?
    auto& pending = pendingRequests[to];
    if (pending.count(from)) {
        cout << "Friend request already exists" << endl;
        return false;
    }

    pending.insert(from);
    cout << "Added friend request from " << from << " to " << to << endl;
    return true;
}

// Accept a friend request from -> to
bool FriendsManager::acceptFriendRequest(const string& from, const string& to) {
    // Validate users exist
    if (users.find(from) == users.end() || users.find(to) == users.end()) {
        throw runtime_error("User not found");
    }

    auto& pending = pendingRequests[to];
    if (!pending.count(from)) {
        return false;
    }

    // Remove from pending requests
    pending.erase(from);

    // Add to both users' friend trees
    try {
        users[to].getFriendTree().insert(from);
        users[from].getFriendTree().insert(to);
    } catch (const exception& e) {
        cerr << "Error adding friends: " << e.what() << endl;
        // Rollback changes
        users[to].getFriendTree().remove(from);
        users[from].getFriendTree().remove(to);
        throw;
    }

    return true;
}

// Reject a friend request from -> to
bool FriendsManager::rejectFriendRequest(const string& from, const string& to) {
    // Validate users exist
    if (users.find(from) == users.end() || users.find(to) == users.end()) {
        throw runtime_error("User not found");
    }

    auto& pending = pendingRequests[to];
    if (!pending.count(from)) {
        return false;
    }

    pending.erase(from);
    return true;
}

// Cancel a friend request sent from 'from' to 'to'
bool FriendsManager::cancelFriendRequest(const string& from, const string& to) {
    // Validate users exist
    if (users.find(from) == users.end() || users.find(to) == users.end()) {
        throw runtime_error("User not found");
    }

    auto& pending = pendingRequests[to];
    if (!pending.count(from)) {
        return false;
    }

    pending.erase(from);
    return true;
}

// Remove 'friendName' from 'username's friend list and vice versa
bool FriendsManager::removeFriend(const string& username, const string& friendName) {
    // Validate users exist
    if (users.find(username) == users.end() || users.find(friendName) == users.end()) {
        throw runtime_error("User not found");
    }

    // Check if they are friends
    if (!areFriends(username, friendName)) {
        return false;
    }

    try {
        users[username].getFriendTree().remove(friendName);
        users[friendName].getFriendTree().remove(username);
    } catch (const exception& e) {
        cerr << "Error removing friends: " << e.what() << endl;
        throw;
    }

    return true;
}

// Get a list of friends (in-order traversal of AVL tree)
vector<string> FriendsManager::getFriendList(const string& username) const {
    if (users.find(username) == users.end()) {
        throw runtime_error("User not found");
    }
    return users.at(username).getFriendTree().inOrder();
}

// Get pending requests for a user
vector<string> FriendsManager::getPendingRequests(const string& username) const {
    if (users.find(username) == users.end()) {
        throw runtime_error("User not found");
    }
    if (!pendingRequests.count(username)) return {};
    const auto& set = pendingRequests.at(username);
    return vector<string>(set.begin(), set.end());
}

// Check if two users are friends
bool FriendsManager::areFriends(const string& userA, const string& userB) const {
    if (users.find(userA) == users.end() || users.find(userB) == users.end()) {
        throw runtime_error("User not found");
    }
    return users.at(userA).getFriendTree().contains(userB);
}

// Get mutual friends
vector<string> FriendsManager::getMutualFriends(const string& userA, const string& userB) const {

    const auto listA = users.at(userA).getFriendTree().inOrder();
    const auto listB = users.at(userB).getFriendTree().inOrder();

    unordered_set<string> setA(listA.begin(), listA.end());
    vector<string> mutual;

    for (int i = 0; i < listB.size(); i++) {
    string name = listB[i];
    if (setA.count(name)) {
        mutual.push_back(name);
    }
}
    return mutual;
}

// Suggest friends based on 2nd-degree connections
vector<string> FriendsManager::suggestFriends(const string& username) const {
    if (!users.count(username)) return {};

    const auto& user = users.at(username);
    const auto directFriends = user.getFriendTree().inOrder();
    unordered_set<string> suggestionsSet;

    for (int i =0; i< directFriends.size(); i++) {
        string friendName = directFriends[i];
        const auto secondDegree = users.at(friendName).getFriendTree().inOrder();
        for (int j = 0; j < secondDegree.size(); j++) {
            string potential = secondDegree[j];
            if (potential == username) continue;
            if (!user.getFriendTree().contains(potential)) {
                suggestionsSet.insert(potential);
            }
        }
    }

    return vector<string>(suggestionsSet.begin(), suggestionsSet.end());
}

// Save all friends to a JSON file
void FriendsManager::saveFriends(const std::string& filename) {
    try {
        // Create directory if it doesn't exist
        fs::path filePath(filename);
        fs::create_directories(filePath.parent_path());

        json j;
        for (const auto& pair : users) {
            const string& username = pair.first;
            vector<string> friends = pair.second.getFriendTree().inOrder();
            j[username] = friends;
        }

        ofstream file(filename);
        if (!file.is_open()) {
            throw runtime_error("Failed to open friends file for writing: " + filename);
        }

        file << j.dump(4);
        file.close();

        if (file.fail()) {
            throw runtime_error("Failed to write to friends file: " + filename);
        }

        cout << "Successfully saved friends to: " << filename << endl;
        cout << "Current friends:" << endl;
        for (const auto& pair : users) {
            cout << "  " << pair.first << " has friends: ";
            for (const auto& friend_name : pair.second.getFriendTree().inOrder()) {
                cout << friend_name << " ";
            }
            cout << endl;
        }
    } catch (const exception& e) {
        cerr << "Error saving friends: " << e.what() << endl;
        throw;
    }
}

// Load all friends from a JSON file
void FriendsManager::loadFriends(const std::string& filename) {
    try {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Warning: Friends file does not exist: " << filename << endl;
            return;
        }

        json j;
        file >> j;

        cout << "Loading friends from: " << filename << endl;
        for (auto it = j.begin(); it != j.end(); ++it) {
            const string& username = it.key();
            if (users.find(username) == users.end()) {
                cerr << "Warning: Skipping unknown user: " << username << endl;
                continue;
            }

            const json& friendList = it.value();
            cout << "  Loading friends for " << username << ": ";
            for (const auto& friendName : friendList) {
                if (users.find(friendName) != users.end()) {
                    users[username].getFriendTree().insert(friendName);
                    cout << friendName << " ";
                } else {
                    cerr << "\n  Warning: Skipping unknown friend: " << friendName << " for user: " << username << endl;
                }
            }
            cout << endl;
        }
    } catch (const exception& e) {
        cerr << "Error loading friends: " << e.what() << endl;
        throw;
    }
}

// Save pending requests to a JSON file
void FriendsManager::savePendingRequests(const std::string& filename) {
    try {
        // Create directory if it doesn't exist
        fs::path filePath(filename);
        fs::create_directories(filePath.parent_path());

        json j;
        for (const auto& pair : pendingRequests) {
            const string& receiver = pair.first;
            const unordered_set<string>& senders = pair.second;
            j[receiver] = vector<string>(senders.begin(), senders.end());
        }

        ofstream file(filename);
        if (!file.is_open()) {
            throw runtime_error("Failed to open pending requests file for writing: " + filename);
        }

        file << j.dump(4);
        file.close();

        if (file.fail()) {
            throw runtime_error("Failed to write to pending requests file: " + filename);
        }

        cout << "Successfully saved pending requests to: " << filename << endl;
        cout << "Current pending requests:" << endl;
        for (const auto& pair : pendingRequests) {
            cout << "  " << pair.first << " has requests from: ";
            for (const auto& sender : pair.second) {
                cout << sender << " ";
            }
            cout << endl;
        }
    } catch (const exception& e) {
        cerr << "Error saving pending requests: " << e.what() << endl;
        throw;
    }
}

// Load pending requests from a JSON file
void FriendsManager::loadPendingRequests(const std::string& filename) {
    try {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Warning: Pending requests file does not exist: " << filename << endl;
            return;
        }

        json j;
        file >> j;

        cout << "Loading pending requests from: " << filename << endl;
        for (auto it = j.begin(); it != j.end(); ++it) {
            const string& receiver = it.key();
            if (users.find(receiver) == users.end()) {
                cerr << "Warning: Skipping unknown receiver: " << receiver << endl;
                continue;
            }

            const json& senders = it.value();
            cout << "  Loading requests for " << receiver << ": ";
            for (const auto& sender : senders) {
                if (users.find(sender) != users.end()) {
                    pendingRequests[receiver].insert(sender);
                    cout << sender << " ";
                } else {
                    cerr << "\n  Warning: Skipping unknown sender: " << sender << " for receiver: " << receiver << endl;
                }
            }
            cout << endl;
        }
    } catch (const exception& e) {
        cerr << "Error loading pending requests: " << e.what() << endl;
        throw;
    }
}

int FriendsManager::getFriendCount(const std::string& username) const {
    if (users.find(username) == users.end()) {
        throw runtime_error("User not found");
    }
    return users.at(username).getFriendTree().size();
}