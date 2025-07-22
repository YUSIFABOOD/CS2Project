#include "include/UserSearchBST.h"
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <iostream>

UserSearchBST::UserSearchBST() : root(nullptr) {}

std::shared_ptr<UserNode> UserSearchBST::insert(std::shared_ptr<UserNode> node, const std::string& username) {
    if (username.empty()) {
        throw std::invalid_argument("Username cannot be empty");
    }

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
    try {
        if (!username.empty()) {
            root = insert(root, username);
            std::cout << "Added user to search BST: " << username << std::endl;
        } else {
            throw std::invalid_argument("Cannot insert empty username");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error inserting user into search BST: " << e.what() << std::endl;
        throw;
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
    if (!node || prefix.empty()) return;
    
    // Convert to lowercase for case-insensitive search
    std::string lowerUsername = node->username;
    std::string lowerPrefix = prefix;
    std::transform(lowerUsername.begin(), lowerUsername.end(), lowerUsername.begin(), ::tolower);
    std::transform(lowerPrefix.begin(), lowerPrefix.end(), lowerPrefix.begin(), ::tolower);
    
    // Check if current node matches prefix
    if (lowerUsername.substr(0, lowerPrefix.length()) == lowerPrefix) {
        result.push_back(node->username);
        std::cout << "Found prefix match: " << node->username << std::endl;
    }
    
    // If prefix is less than or equal to current node, search left subtree
    if (lowerPrefix <= lowerUsername) {
        searchPrefix(node->left, prefix, result);
    }
    
    // Always search right subtree as it might contain matching prefixes
    searchPrefix(node->right, prefix, result);
}

std::vector<std::string> UserSearchBST::searchByPrefix(const std::string& prefix) const {
    std::vector<std::string> result;
    try {
        if (!prefix.empty()) {
            std::cout << "Starting prefix search for: '" << prefix << "'" << std::endl;
            searchPrefix(root, prefix, result);
            std::cout << "Prefix search completed. Found " << result.size() << " matches." << std::endl;
        } else {
            std::cout << "Empty prefix provided, returning empty result" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in prefix search: " << e.what() << std::endl;
        throw;
    }
    return result;
}

void UserSearchBST::searchSubstring(std::shared_ptr<UserNode> node, const std::string& query, std::vector<std::string>& result) const {
    if (!node || query.empty()) return;
    
    // Convert to lowercase for case-insensitive search
    std::string lowerUsername = node->username;
    std::string lowerQuery = query;
    std::transform(lowerUsername.begin(), lowerUsername.end(), lowerUsername.begin(), ::tolower);
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    // Check if current node contains the query substring
    if (lowerUsername.find(lowerQuery) != std::string::npos) {
        result.push_back(node->username);
        std::cout << "Found substring match: " << node->username << std::endl;
    }
    
    // Search both subtrees as substring matches could be anywhere
    searchSubstring(node->left, query, result);
    searchSubstring(node->right, query, result);
}

std::vector<std::string> UserSearchBST::searchBySubstring(const std::string& query) const {
    std::vector<std::string> result;
    try {
        if (!query.empty()) {
            std::cout << "Starting substring search for: '" << query << "'" << std::endl;
            searchSubstring(root, query, result);
            std::cout << "Substring search completed. Found " << result.size() << " matches." << std::endl;
        } else {
            std::cout << "Empty query provided, returning empty result" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in substring search: " << e.what() << std::endl;
        throw;
    }
    return result;
}

bool UserSearchBST::userExists(const std::string& username) const {
    if (username.empty()) {
        return false;
    }

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
    try {
        clear();
        std::cout << "Rebuilding search BST with " << users.size() << " users" << std::endl;
        for (const auto& user : users) {
            insertUser(user);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error rebuilding search BST: " << e.what() << std::endl;
        throw;
    }
}
