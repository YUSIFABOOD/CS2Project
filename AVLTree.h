#ifndef AVLTREE_H
#define AVLTREE_H
#include <bits/stdc++.h>
using namespace std;
class AVLtree {
public:
    struct Nodeuser {
        string username;
        int height;
        vector<string> friends;
        int mutualCount;

        Nodeuser* left;
        Nodeuser* right;
        Nodeuser* parent;

        Nodeuser(const string& name)
            : username(name), height(1), mutualCount(0),
              left(nullptr), right(nullptr), parent(nullptr) {}
    };

    Nodeuser* root;
    string referenceUsername;

    AVLtree() : root(nullptr) {}

    int getHeight(Nodeuser* node) {
        return node ? node->height : 0;
    }

    int getBalance(Nodeuser* node) {
        return node ? getHeight(node->left) - getHeight(node->right) : 0;
    }

    void updateHeight(Nodeuser* node) {
        if (node) {
            node->height = 1 + max(getHeight(node->left), getHeight(node->right));
        }
    }

    Nodeuser* rotateRight(Nodeuser* y) {
        Nodeuser* x = y->left;
        Nodeuser* T2 = x->right;

        x->right = y;
        y->left = T2;
        updateHeight(y);
        updateHeight(x);
        return x;
    }

    Nodeuser* rotateLeft(Nodeuser* x) {
        Nodeuser* y = x->right;
        Nodeuser* T2 = y->left;

        y->left = x;
        x->right = T2;

        updateHeight(x);
        updateHeight(y);
        return y;
    }

    int countMutual(const vector<string>& a, const vector<string>& b) {
        int count = 0;
        for (const string& friendA : a) {
            for (const string& friendB : b) {
                if (friendA == friendB) {
                    count++;
                    break;
                }
            }
        }
        return count;
    }

    Nodeuser* insertByMutualFriends(Nodeuser* node, Nodeuser* newUser, const vector<string>& referenceFriends) {
        newUser->mutualCount = countMutual(newUser->friends, referenceFriends);

        if (!node) {
            return newUser;
        }

        if (newUser->mutualCount < node->mutualCount ||
            (newUser->mutualCount == node->mutualCount && newUser->username < node->username)) {
            node->left = insertByMutualFriends(node->left, newUser, referenceFriends);
            if (node->left) node->left->parent = node;
        }
        else if (newUser->mutualCount > node->mutualCount ||
                 (newUser->mutualCount == node->mutualCount && newUser->username > node->username)) {
            node->right = insertByMutualFriends(node->right, newUser, referenceFriends);
            if (node->right) node->right->parent = node;
        }
        else {
            return node;
        }

        updateHeight(node);

        int balance = getBalance(node);

        if (balance > 1 && (newUser->mutualCount < node->left->mutualCount ||
                           (newUser->mutualCount == node->left->mutualCount &&
                            newUser->username < node->left->username))) {
            return rotateRight(node);
        }


        if (balance < -1 && (newUser->mutualCount > node->right->mutualCount ||
                            (newUser->mutualCount == node->right->mutualCount &&
                             newUser->username > node->right->username))) {
            return rotateLeft(node);
        }

        if (balance > 1 && (newUser->mutualCount > node->left->mutualCount ||
                           (newUser->mutualCount == node->left->mutualCount &&
                            newUser->username > node->left->username))) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }

        if (balance < -1 && (newUser->mutualCount < node->right->mutualCount ||
                            (newUser->mutualCount == node->right->mutualCount &&
                             newUser->username < node->right->username))) {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }

        return node;
    }
};

#endif