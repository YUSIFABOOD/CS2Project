#ifndef USER_SEARCH_BST_H
#define USER_SEARCH_BST_H

#include <string>
#include <vector>
#include <memory>

struct UserNode {
    std::string username;
    std::shared_ptr<UserNode> left;
    std::shared_ptr<UserNode> right;
    
    UserNode(const std::string& name) : username(name), left(nullptr), right(nullptr) {}
};

class UserSearchBST {
private:
    std::shared_ptr<UserNode> root;
    
    // Helper functions
    std::shared_ptr<UserNode> insert(std::shared_ptr<UserNode> node, const std::string& username);
    void inorderTraversal(std::shared_ptr<UserNode> node, std::vector<std::string>& result) const;
    void searchPrefix(std::shared_ptr<UserNode> node, const std::string& prefix, std::vector<std::string>& result) const;
    void searchSubstring(std::shared_ptr<UserNode> node, const std::string& query, std::vector<std::string>& result) const;
    
public:
    UserSearchBST();
    ~UserSearchBST() = default;
    
    // Core operations
    void insertUser(const std::string& username);
    std::vector<std::string> getAllUsers() const;
    std::vector<std::string> searchByPrefix(const std::string& prefix) const;
    std::vector<std::string> searchBySubstring(const std::string& query) const;
    bool userExists(const std::string& username) const;
    void clear();
    
    // Rebuild BST from user list
    void rebuildFromUsers(const std::vector<std::string>& users);
};

#endif // USER_SEARCH_BST_H
