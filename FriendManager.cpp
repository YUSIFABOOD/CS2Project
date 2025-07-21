#include "include/FriendManager.h"
#include "include/AVLTree.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
using namespace std;

// Constructor: initializes with reference to existing user map
FriendManager::FriendManager(unordered_map<string, User>& usersMap)
    : users(usersMap) {}

// Send a friend request from -> to
bool FriendManager::sendFriendRequest(const string& from, const string& to) {
    if (from == to) return false;
    if (!users.count(from) || !users.count(to)) return false;

    // Already friends?
    if (areFriends(from, to)) return false;

    // Already requested?
    auto& pending = pendingRequests[to];
    if (pending.count(from)) return false;

    pending.insert(from);
    return true;
}

// Accept a friend request from -> to
bool FriendManager::acceptFriendRequest(const string& from, const string& to) {
    if (!users.count(from) || !users.count(to)) return false;

    auto& pending = pendingRequests[to];
    if (!pending.count(from)) return false;

    pending.erase(from);

    users[to].getFriendTree().insert(from);
    users[from].getFriendTree().insert(to);
    return true;
}

// Reject a friend request from -> to
bool FriendManager::rejectFriendRequest(const string& from, const string& to) {
    if (!users.count(from) || !users.count(to)) return false;

    auto& pending = pendingRequests[to];
    if (!pending.count(from)) return false;

    pending.erase(from);
    return true;
}

// Cancel a friend request sent from 'from' to 'to'
bool FriendManager::cancelFriendRequest(const string& from, const string& to) {
    if (!users.count(from) || !users.count(to)) return false;

    auto& pending = pendingRequests[to];
    if (!pending.count(from)) return false; // No such pending request

    pending.erase(from);
    return true;
}

// Remove 'friendName' from 'username's friend list and vice versa
bool FriendManager::removeFriend(const string& username, const string& friendName) {
    if (!users.count(username) || !users.count(friendName)) return false;

    // Check if they are friends
    if (!areFriends(username, friendName)) return false;

    users[username].getFriendTree().remove(friendName); // assumes AVLTree has remove()
    users[friendName].getFriendTree().remove(username);

    return true;
}

// Get a list of friends (in-order traversal of AVL tree)
vector<string> FriendManager::getFriendList(const string& username) const {
    if (!users.count(username)) return {};
    return users.at(username).getFriendTree().inOrder(); // assumes AVLTree has inOrder()
}

// Get pending requests for a user
vector<string> FriendManager::getPendingRequests(const string& username) const {
    if (!pendingRequests.count(username)) return {};
    const auto& set = pendingRequests.at(username);
    return vector<string>(set.begin(), set.end());
}

// Check if two users are friends
bool FriendManager::areFriends(const string& userA, const string& userB) const {
    if (!users.count(userA) || !users.count(userB)) return false;
    return users.at(userA).getFriendTree().contains(userB); // assumes AVLTree has search()
}

// Get mutual friends
vector<string> FriendManager::getMutualFriends(const string& userA, const string& userB) const {
    if (!users.count(userA) || !users.count(userB)) return {};

    const auto listA = users.at(userA).getFriendTree().inOrder();
    const auto listB = users.at(userB).getFriendTree().inOrder();

    unordered_set<string> setA(listA.begin(), listA.end());
    vector<string> mutual;

    for (const auto& name : listB) {
        if (setA.count(name)) {
            mutual.push_back(name);
        }
    }

    return mutual;
}

// Suggest friends based on 2nd-degree connections
vector<string> FriendManager::suggestFriends(const string& username) const {
    if (!users.count(username)) return {};

    const auto& user = users.at(username);
    const auto directFriends = user.getFriendTree().inOrder();
    unordered_set<string> suggestionsSet;

    for (const auto& friendName : directFriends) {
        if (!users.count(friendName)) continue;
        const auto secondDegree = users.at(friendName).getFriendTree().inOrder();
        for (const auto& potential : secondDegree) {
            if (potential == username) continue;
            if (!user.getFriendTree().contains(potential)) {
                suggestionsSet.insert(potential);
            }
        }
    }

    return vector<string>(suggestionsSet.begin(), suggestionsSet.end());
}

// Save all friends to CSV
void FriendManager::saveFriends(const std::string& filename) {
    std::ofstream file(filename);
    for (const auto& [username, user] : users) {
        file << username;
        auto friends = user.getFriendTree().inOrder();
        for (const auto& f : friends) {
            file << "," << f;
        }
        file << "\n";
    }
}

// Load all friends from CSV
void FriendManager::loadFriends(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string username;
        std::getline(ss, username, ',');
        if (!users.count(username)) continue;
        std::string friendName;
        while (std::getline(ss, friendName, ',')) {
            users[username].getFriendTree().insert(friendName);
        }
    }
}

// Save all pending requests to CSV
void FriendManager::savePendingRequests(const std::string& filename) {
    std::ofstream file(filename);
    for (const auto& [receiver, senders] : pendingRequests) {
        file << receiver;
        for (const auto& sender : senders) {
            file << "," << sender;
        }
        file << "\n";
    }
}

// Load all pending requests from CSV
void FriendManager::loadPendingRequests(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string receiver;
        std::getline(ss, receiver, ',');
        std::string sender;
        while (std::getline(ss, sender, ',')) {
            pendingRequests[receiver].insert(sender);
        }
    }
}

int FriendManager::getFriendCount(const std::string& username) const {
    if (!users.count(username)) return 0;
    return users.at(username).getFriendTree().size(); // assumes AVLTree has size()
}
