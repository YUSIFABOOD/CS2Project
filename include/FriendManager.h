#ifndef FRIEND_MANAGER_H
#define FRIEND_MANAGER_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Users.h" // Make sure this includes User and its AVLTree
#include <fstream>
#include <sstream>

class FriendManager {
private:
    // Reference to the global user storage (shared from Authentication or main)
    std::unordered_map<std::string, User>& users;

    // Keeps track of pending friend requests: toUser -> set of usernames who sent requests
    std::unordered_map<std::string, std::unordered_set<std::string>> pendingRequests;

public:
    // Constructor takes reference to existing user storage
    FriendManager(std::unordered_map<std::string, User>& usersMap);

    // Core friendship operations
    bool sendFriendRequest(const std::string& from, const std::string& to);
    bool acceptFriendRequest(const std::string& from, const std::string& to);
    bool rejectFriendRequest(const std::string& from, const std::string& to);
    bool cancelFriendRequest(const std::string& from, const std::string& to);
    bool removeFriend(const std::string& username, const std::string& friendName);

    // Info and utility functions
    std::vector<std::string> getFriendList(const std::string& username) const;
    std::vector<std::string> getPendingRequests(const std::string& username) const;
    bool areFriends(const std::string& userA, const std::string& userB) const;

    // Mutual and suggestion logic
    std::vector<std::string> getMutualFriends(const std::string& userA, const std::string& userB) const;
    std::vector<std::string> suggestFriends(const std::string& username) const;

    // Save and load functions
    void saveFriends(const std::string& filename);
    void loadFriends(const std::string& filename);
    void savePendingRequests(const std::string& filename);
    void loadPendingRequests(const std::string& filename);

    int getFriendCount(const std::string& username) const;
};

#endif // FRIEND_MANAGER_H
