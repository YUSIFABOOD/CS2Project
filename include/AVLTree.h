#ifndef AVLTREE_H
#define AVLTREE_H

#include <iostream>
#include <vector>
#include <algorithm>

template <typename T>
class AVLTree {
private:
    struct Node {
        T data;
        Node* left;
        Node* right;
        int height;

        Node(const T& val) : data(val), left(nullptr), right(nullptr), height(1) {}
    };

    Node* root;

    int height(Node* node) const {
        return node ? node->height : 0;
    }

    int getBalance(Node* node) const {
        return node ? height(node->left) - height(node->right) : 0;
    }

    Node* rightRotate(Node* y) {
        Node* x = y->left;
        Node* T2 = x->right;

        x->right = y;
        y->left = T2;

        y->height = std::max(height(y->left), height(y->right)) + 1;
        x->height = std::max(height(x->left), height(x->right)) + 1;

        return x;
    }

    Node* leftRotate(Node* x) {
        Node* y = x->right;
        Node* T2 = y->left;

        y->left = x;
        x->right = T2;

        x->height = std::max(height(x->left), height(x->right)) + 1;
        y->height = std::max(height(y->left), height(y->right)) + 1;

        return y;
    }

    Node* insert(Node* node, const T& key) {
        if (!node) return new Node(key);

        if (key < node->data)
            node->left = insert(node->left, key);
        else if (key > node->data)
            node->right = insert(node->right, key);
        else
            return node; // duplicate, do nothing

        node->height = 1 + std::max(height(node->left), height(node->right));
        int balance = getBalance(node);

        // Rebalance if needed
        if (balance > 1 && key < node->left->data)
            return rightRotate(node);
        if (balance < -1 && key > node->right->data)
            return leftRotate(node);
        if (balance > 1 && key > node->left->data) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }
        if (balance < -1 && key < node->right->data) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        return node;
    }

    Node* remove(Node* node, const T& key) {
        if (!node) return nullptr;

        if (key < node->data)
            node->left = remove(node->left, key);
        else if (key > node->data)
            node->right = remove(node->right, key);
        else {
            if (!node->left || !node->right) {
                Node* temp = node->left ? node->left : node->right;
                delete node;
                return temp;
            }

            Node* temp = node->right;
            while (temp->left)
                temp = temp->left;

            node->data = temp->data;
            node->right = remove(node->right, temp->data);
        }

        node->height = 1 + std::max(height(node->left), height(node->right));
        int balance = getBalance(node);

        // Rebalance
        if (balance > 1 && getBalance(node->left) >= 0)
            return rightRotate(node);
        if (balance > 1 && getBalance(node->left) < 0) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }
        if (balance < -1 && getBalance(node->right) <= 0)
            return leftRotate(node);
        if (balance < -1 && getBalance(node->right) > 0) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        return node;
    }

    bool contains(Node* node, const T& key) const {
        if (!node) return false;
        if (key == node->data) return true;
        if (key < node->data) return contains(node->left, key);
        return contains(node->right, key);
    }

    void inOrder(Node* node, std::vector<T>& result) const {
        if (!node) return;
        inOrder(node->left, result);
        result.push_back(node->data);
        inOrder(node->right, result);
    }

    void destroy(Node* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }
    int size(Node* node) const {
        if (!node) return 0;
        return 1 + size(node->left) + size(node->right);
    }

public:
    AVLTree() : root(nullptr) {}

    ~AVLTree() {
        destroy(root);
    }

    void insert(const T& key) {
        root = insert(root, key);
    }

    void remove(const T& key) {
        root = remove(root, key);
    }

    bool contains(const T& key) const {
        return contains(root, key);
    }

    std::vector<T> inOrder() const {
        std::vector<T> result;
        inOrder(root, result);
        return result;
    }

    int size() const {
        return size(root);
    }

    
};

#endif