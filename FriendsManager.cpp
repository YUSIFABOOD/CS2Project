#include "include/FriendsManager.h"
#include "include/AVLTree.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>
using namespace std;
using json = nlohmann::json;

// Constructor: initializes with reference to existing user map
FriendsManager::FriendsManager(unordered_map<string, User>& usersMap)
    : users(usersMap) {}

// Send a friend request from -> to
bool FriendsManager::sendFriendRequest(const string& from, const string& to) {

    // Already friends?
    if (areFriends(from, to)) return false;

    // Already requested?
    auto& pending = pendingRequests[to];
    if (pending.count(from)) return false;

    pending.insert(from);
    return true;
}

// Accept a friend request from -> to
bool FriendsManager::acceptFriendRequest(const string& from, const string& to) {

    auto& pending = pendingRequests[to];
    if (!pending.count(from)) return false;

    pending.erase(from);

    users[to].getFriendTree().insert(from);
    users[from].getFriendTree().insert(to);
    return true;
}

// Reject a friend request from -> to
bool FriendsManager::rejectFriendRequest(const string& from, const string& to) {

    auto& pending = pendingRequests[to];
    if (!pending.count(from)) return false;

    pending.erase(from);
    return true;
}

// Cancel a friend request sent from 'from' to 'to'
bool FriendsManager::cancelFriendRequest(const string& from, const string& to) {

    auto& pending = pendingRequests[to];
    if (!pending.count(from)) return false; // No such pending request

    pending.erase(from);
    return true;
}

// Remove 'friendName' from 'username's friend list and vice versa
bool FriendsManager::removeFriend(const string& username, const string& friendName) {

    // Check if they are friends
    if (!areFriends(username, friendName)) return false;

    users[username].getFriendTree().remove(friendName); // assumes AVLTree has remove()
    users[friendName].getFriendTree().remove(username);

    return true;
}

// Get a list of friends (in-order traversal of AVL tree)
vector<string> FriendsManager::getFriendList(const string& username) const {
    return users.at(username).getFriendTree().inOrder(); // assumes AVLTree has inOrder()
}

// Get pending requests for a user
vector<string> FriendsManager::getPendingRequests(const string& username) const {
    if (!pendingRequests.count(username)) return {};
    const auto& set = pendingRequests.at(username);
    return vector<string>(set.begin(), set.end());
}

// Check if two users are friends
bool FriendsManager::areFriends(const string& userA, const string& userB) const {
    return users.at(userA).getFriendTree().contains(userB); // assumes AVLTree has search()
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
    json j;

    for (const auto& pair : users) {
        const std::string& username = pair.first;
        const auto& user = pair.second;
        std::vector<std::string> friends = user.getFriendTree().inOrder();

        j[username] = friends;
    }

    std::ofstream file(filename);
    file << j.dump(4); // Pretty-print with 4-space indentation
}

// Load all friends from a JSON file
void FriendsManager::loadFriends(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return;

    json j;
    file >> j;

    for (auto it = j.begin(); it != j.end(); ++it) {
        const std::string& username = it.key();
        const json& friendList = it.value();

        if (!users.count(username)) continue;

        for (int i = 0; i < friendList.size(); i++) {
            users[username].getFriendTree().insert(friendList[i]);
        }
    }
}

// Save pending requests to a JSON file
void FriendsManager::savePendingRequests(const std::string& filename) {
    json j;

    for (const auto& pair : pendingRequests) {
        const std::string& receiver = pair.first;
        const std::unordered_set<std::string>& senders = pair.second;

        std::vector<std::string> sendersVec(senders.begin(), senders.end());
        j[receiver] = sendersVec;
    }

    std::ofstream file(filename);
    file << j.dump(4);
}

// Load pending requests from a JSON file
void FriendsManager::loadPendingRequests(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return;

    json j;
    file >> j;

    for (auto it = j.begin(); it != j.end(); ++it) {
        const std::string& receiver = it.key();
        const json& senders = it.value();

        for (int i = 0; i < senders.size(); i++) {
            pendingRequests[receiver].insert(senders[i]);
        }
    }
}
int FriendsManager::getFriendCount(const std::string& username) const {
    if (!users.count(username)) return 0;
    return users.at(username).getFriendTree().size(); // assumes AVLTree has size()
}