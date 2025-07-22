#ifndef BST_H
#define BST_H

#include <bits/stdc++.h>
using namespace std;

class BST {
public:
    struct Node {
        string username;
        Node* parent;
        Node* left;
        Node* right;

        Node(const string& name, Node* p = nullptr)
            : username(name), parent(p), left(nullptr), right(nullptr) {}
    };

    Node* root;

    BST() : root(nullptr) {}


    Node* insert(Node* node, const string& username, Node* parent = nullptr) {
        if (!node) {
            Node* newNode = new Node(username, parent);
            return newNode;
        }

        if (username < node->username) {
            node->left = insert(node->left, username, node);
        } else if (username > node->username) {
            node->right = insert(node->right, username, node);
        }

        return node;
    }


    void searchByPrefix(Node* node, const string& prefix, vector<Node*>& result) {
        if (!node) return;

        if (node->username.compare(0, prefix.size(), prefix) == 0) {
            result.push_back(node);
        }

        if (prefix <= node->username)
            searchByPrefix(node->left, prefix, result);
        if (prefix >= node->username)
            searchByPrefix(node->right, prefix, result);
    }

};

#endif
