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
    Nodeuser* referenceUser;

    AVLtree() : root(nullptr), referenceUser(nullptr) {}

    void setReferenceUser(Nodeuser* user) {
        referenceUser = user;
    }

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

    Nodeuser* insert(Nodeuser* root, Nodeuser* newUser) {
        if (!root) {
            newUser->mutualCount = countMutual(newUser->friends, referenceUser->friends);
            return newUser;
        }

        int newCount = countMutual(newUser->friends, referenceUser->friends);
        int rootCount = countMutual(root->friends, referenceUser->friends);

        if (newCount > rootCount) {
            root->left = insert(root->left, newUser);
            if (root->left) root->left->parent = root;
        } else if (newCount < rootCount) {
            root->right = insert(root->right, newUser);
            if (root->right) root->right->parent = root;
        } else {

            if (newUser->username < root->username) {
                root->left = insert(root->left, newUser);
                if (root->left) root->left->parent = root;
            } else if (newUser->username > root->username) {
                root->right = insert(root->right, newUser);
                if (root->right) root->right->parent = root;
            } else {

                return root;
            }
        }

        updateHeight(root);
        int balance = getBalance(root);


        if (balance > 1 && getBalance(root->left) >= 0)
            return rotateRight(root);

        if (balance < -1 && getBalance(root->right) <= 0)
            return rotateLeft(root);


        if (balance > 1 && getBalance(root->left) < 0) {
            root->left = rotateLeft(root->left);
            return rotateRight(root);
        }

        if (balance < -1 && getBalance(root->right) > 0) {
            root->right = rotateRight(root->right);
            return rotateLeft(root);
        }

        return root;
    }


Nodeuser* minValueNode(Nodeuser* node) {
    Nodeuser* current = node;
    while (current && current->right != nullptr) {
        current = current->right;
    }
    return current;
}


Nodeuser* deleteUser(Nodeuser* root, int keyMutualCount, const string& keyUsername) {
    if (!root) return nullptr;

    if (keyMutualCount > root->mutualCount) {
        root->left = deleteUser(root->left, keyMutualCount, keyUsername);
    } else if (keyMutualCount < root->mutualCount) {
        root->right = deleteUser(root->right, keyMutualCount, keyUsername);
    } else {

        if (keyUsername < root->username) {
            root->left = deleteUser(root->left, keyMutualCount, keyUsername);
        } else if (keyUsername > root->username) {
            root->right = deleteUser(root->right, keyMutualCount, keyUsername);
        } else {

            if (!root->left || !root->right) {
                Nodeuser* temp = root->left ? root->left : root->right;
                delete root;
                return temp;
            } else {
                Nodeuser* temp = minValueNode(root->right);
                 root->username = temp->username;
                root->friends = temp->friends;
                root->mutualCount = temp->mutualCount;
               root->right = deleteUser(root->right, temp->mutualCount, temp->username);
            }
        }
    }

    updateHeight(root);
    int balance = getBalance(root);

    if (balance > 1 && getBalance(root->left) >= 0)
        return rotateRight(root);

    if (balance > 1 && getBalance(root->left) < 0) {
        root->left = rotateLeft(root->left);
        return rotateRight(root);
    }

    if (balance < -1 && getBalance(root->right) <= 0)
        return rotateLeft(root);

    if (balance < -1 && getBalance(root->right) > 0) {
        root->right = rotateRight(root->right);
        return rotateLeft(root);
    }

    return root;
}






};


#endif


