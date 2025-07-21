#include "include/UserSearchBST.h"
#include <algorithm>
#include <cctype>

UserSearchBST::UserSearchBST() : root(nullptr) {}

std::shared_ptr<UserNode> UserSearchBST::insert(std::shared_ptr<UserNode> node, const std::string& username) {
    if (!node) {
        return std::make_shared<UserNode>(username);
    }
    
    // Convert to lowercase for case-insensitive comparison
    std::string lowerUsername = username;
    std::string lowerNodeName = node->username;
    std::transform(lowerUsername.begin(), lowerUsername.end(), lowerUsername.begin(), ::tolower);
    std::transform(lowerNodeName.begin(), lowerNodeName.end(), lowerNodeName.begin(), ::tolower);
    
    if (lowerUsername < lowerNodeName) {
        node->left = insert(node->left, username);
    } else if (lowerUsername > lowerNodeName) {
        node->right = insert(node->right, username);
    }
    // If equal, don't insert duplicate
    
    return node;
}

void UserSearchBST::insertUser(const std::string& username) {
    if (!username.empty()) {
        root = insert(root, username);
    }
}

void UserSearchBST::inorderTraversal(std::shared_ptr<UserNode> node, std::vector<std::string>& result) const {
    if (node) {
        inorderTraversal(node->left, result);
        result.push_back(node->username);
        inorderTraversal(node->right, result);
    }
}

std::vector<std::string> UserSearchBST::getAllUsers() const {
    std::vector<std::string> result;
    inorderTraversal(root, result);
    return result;
}

void UserSearchBST::searchPrefix(std::shared_ptr<UserNode> node, const std::string& prefix, std::vector<std::string>& result) const {
    if (!node) return;
    
    // Convert to lowercase for case-insensitive search
    std::string lowerUsername = node->username;
    std::string lowerPrefix = prefix;
    std::transform(lowerUsername.begin(), lowerUsername.end(), lowerUsername.begin(), ::tolower);
    std::transform(lowerPrefix.begin(), lowerPrefix.end(), lowerPrefix.begin(), ::tolower);
    
    // Check if current node matches prefix
    if (lowerUsername.substr(0, lowerPrefix.length()) == lowerPrefix) {
        result.push_back(node->username);
    }
    
    // Decide which subtrees to search
    if (lowerPrefix <= lowerUsername) {
        searchPrefix(node->left, prefix, result);
    }
    if (lowerPrefix >= lowerUsername.substr(0, lowerPrefix.length())) {
        searchPrefix(node->right, prefix, result);
    }
}

std::vector<std::string> UserSearchBST::searchByPrefix(const std::string& prefix) const {
    std::vector<std::string> result;
    if (!prefix.empty()) {
        searchPrefix(root, prefix, result);
    }
    return result;
}

void UserSearchBST::searchSubstring(std::shared_ptr<UserNode> node, const std::string& query, std::vector<std::string>& result) const {
    if (!node) return;
    
    // Convert to lowercase for case-insensitive search
    std::string lowerUsername = node->username;
    std::string lowerQuery = query;
    std::transform(lowerUsername.begin(), lowerUsername.end(), lowerUsername.begin(), ::tolower);
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    // Check if current node contains the query substring
    if (lowerUsername.find(lowerQuery) != std::string::npos) {
        result.push_back(node->username);
    }
    
    // Search both subtrees for substring matches
    searchSubstring(node->left, query, result);
    searchSubstring(node->right, query, result);
}

std::vector<std::string> UserSearchBST::searchBySubstring(const std::string& query) const {
    std::vector<std::string> result;
    if (!query.empty()) {
        searchSubstring(root, query, result);
    }
    return result;
}

bool UserSearchBST::userExists(const std::string& username) const {
    std::shared_ptr<UserNode> current = root;
    
    while (current) {
        std::string lowerUsername = username;
        std::string lowerCurrentName = current->username;
        std::transform(lowerUsername.begin(), lowerUsername.end(), lowerUsername.begin(), ::tolower);
        std::transform(lowerCurrentName.begin(), lowerCurrentName.end(), lowerCurrentName.begin(), ::tolower);
        
        if (lowerUsername == lowerCurrentName) {
            return true;
        } else if (lowerUsername < lowerCurrentName) {
            current = current->left;
        } else {
            current = current->right;
        }
    }
    
    return false;
}

void UserSearchBST::clear() {
    root = nullptr;
}

void UserSearchBST::rebuildFromUsers(const std::vector<std::string>& users) {
    clear();
    for (const auto& user : users) {
        insertUser(user);
    }
}
