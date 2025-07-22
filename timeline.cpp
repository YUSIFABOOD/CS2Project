#include <bits/stdc++.h>
#include <crow.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <ctime>
#include <filesystem>
#include "include/timeline.h"

using namespace std;
namespace fs = std::filesystem;
using json = nlohmann::json;

//---------------------------------------------------
//Definition of comment class methods
//---------------------------------------------------
Comment::Comment(int id, int pId, const string& o, const string& c) 
    : commentId(id), postId(pId), owner(o), content(c), timestamp(time(nullptr)) {}

json Comment::CommentToJson() const {
    return {
        {"id", commentId},
        {"postId", postId},
        {"owner", owner},
        {"content", content},
        {"timestamp", timestamp}
    };
}

Comment Comment::CommentFromJson(const json& j) {
    Comment c(
        j["id"],
        j["postId"],
        j["owner"],
        j["content"]
    );
    c.timestamp = j["timestamp"];
    return c;
}

int Comment::getCommentId() const { return commentId; }
string Comment::getCommentContent() const { return content; }
string Comment::getCommentOwner() const { return owner; }
time_t Comment::getCommentTimes() const { return timestamp; }

void Comment::setContent(const string& c) {
    content = c;
}

void Comment::setTimestamp(time_t newTime) {
    timestamp = newTime;
}

//------------------------------------------------------------
//Definition of Post class methods
//------------------------------------------------------------

int Post::getPostId() const { return id; }
string Post::getPostContent() const { return content; }
string Post::getPostOwner() const { return owner; }
time_t Post::getPostTimes() const { return timestamp; }

Post::Post(int i, const string& c, const string& o)
    : id(i), content(c), owner(o), timestamp(time(nullptr)) {}

void Post::AddComment(const string& comment, const string& username) {
    Comment newComment(nextCommentId++, id, username, comment);
    commentVec.push_back(newComment);
}

void Post::EditComment(const string& newComment, const string& username, int commentId) {
    for (auto& target : commentVec) {
        if (target.getCommentId() == commentId) {
            if (target.getCommentOwner() != username) {
                throw runtime_error("Unauthorized: Cannot edit others' comments");
            }
            target.setContent(newComment);
            target.setTimestamp(time(nullptr));
            return;
        }
    }
    throw runtime_error("Comment not found");
}

void Post::deleteComment(const string& username, int commentId) {
    for (auto it = commentVec.begin(); it != commentVec.end(); ++it) {
        if (it->getCommentId() == commentId) {
            if (it->getCommentOwner() != username) {
                throw runtime_error("Unauthorized: Cannot delete others' comments");
            }
            commentVec.erase(it);
            return;
        }
    }
    throw runtime_error("Comment not found");
}

const vector<Comment>& Post::getComments() const {
    return commentVec;
}

// Reaction methods
void Post::addReaction(const string& username) {
    // Check if user already reacted
    auto it = find(reactions.begin(), reactions.end(), username);
    if (it == reactions.end()) {
        reactions.push_back(username);
    }
}

void Post::removeReaction(const string& username) {
    auto it = find(reactions.begin(), reactions.end(), username);
    if (it != reactions.end()) {
        reactions.erase(it);
    }
}

bool Post::hasReaction(const string& username) const {
    return find(reactions.begin(), reactions.end(), username) != reactions.end();
}

int Post::getReactionCount() const {
    return reactions.size();
}

const vector<string>& Post::getReactions() const {
    return reactions;
}

Post Post::fromJson(const json& j) {
    Post p(j.at("id").get<int>(), j.at("content").get<string>(), j.at("owner").get<string>());
    p.timestamp = j.at("timestamp").get<time_t>();
    if (j.contains("comments")) {
        for (const auto& cj : j.at("comments")) {
            p.commentVec.push_back(Comment::CommentFromJson(cj));
        }
    }
    if (j.contains("reactions")) {
        for (const auto& reaction : j.at("reactions")) {
            p.reactions.push_back(reaction.get<string>());
        }
    }
    // You might want to set the nextCommentId here as well
    int maxCommentId = 0;
    for(const auto& comment : p.commentVec) {
        if (comment.getCommentId() > maxCommentId) {
            maxCommentId = comment.getCommentId();
        }
    }
    p.setNextCommentId(maxCommentId + 1);

    return p;
}

void Post::Edit(const string& newContent) {
    content = newContent;
}

json Post::PostToJson() const {
    json j;
    j["id"] = id;
    j["content"] = content;
    j["owner"] = owner;
    j["timestamp"] = timestamp;

    json comments_json = json::array();
    for (const auto& comment : commentVec) {
        comments_json.push_back(comment.CommentToJson());
    }
    j["comments"] = comments_json;
    
    json reactions_json = json::array();
    for (const auto& reaction : reactions) {
        reactions_json.push_back(reaction);
    }
    j["reactions"] = reactions_json;

    return j;
}

//--------------------------------------------------------------------------
//Definition of posts manager class
//--------------------------------------------------------------------------
PostsManager::PostsManager(const string& file) : filePath(file), nextPostId(1) {
    loadPosts();
}

void PostsManager::Add_post(const string& post, const string& name) {
    Post newPost(nextPostId++, post, name);
    PostsVec.push_back(newPost);
    savePosts();
}

void PostsManager::loadPosts() {
    ifstream file(filePath);
    if (!file.is_open()) {
        return; // File might not exist on first run
    }
    json data;
    // Check if the file is empty before parsing
    file.seekg(0, ios::end);
    if (file.tellg() == 0) {
        // File is empty, skip parsing
        return;
    }
    file.seekg(0, ios::beg);
    file >> data;
    if (data.contains("posts")) {
        PostsVec.clear();
        for (const auto& post_json : data["posts"]) {
            PostsVec.push_back(Post::fromJson(post_json));
        }
    }

    int maxId = 0;
    if (!PostsVec.empty()) {
        maxId = std::max_element(PostsVec.begin(), PostsVec.end(), 
            [](const Post& a, const Post& b) {
                return a.getPostId() < b.getPostId();
            })->getPostId();
    }
    nextPostId = maxId + 1;

}

void PostsManager::savePosts() {
    json posts_json_array = json::array();
    for (const auto& post : PostsVec) {
        posts_json_array.push_back(post.PostToJson());
    }

    json final_json;
    final_json["posts"] = posts_json_array;

    ofstream file(filePath);
    if (file.is_open()) {
        file << final_json.dump(4);
    } else {
        throw runtime_error("Error opening posts file for writing.");
    }
}

void PostsManager::EditPost(int id, string& username, const string& newContent) {
    for (auto& post : PostsVec) {
        if (post.getPostId() == id) {
            if (post.getPostOwner() != username) {
                throw runtime_error("Unauthorized: Cannot edit others' posts");
            }
            post.Edit(newContent);
            savePosts();
            return;
        }
    }
    throw runtime_error("Post not found");
}

void PostsManager::deletePost(int id) {
    auto it = find_if(PostsVec.begin(), PostsVec.end(),
        [id](const Post& p) { return p.getPostId() == id; });
    if (it != PostsVec.end()) {
        PostsVec.erase(it);
        savePosts();
    } else {
        throw runtime_error("Post not found");
    }
}

vector<Post>& PostsManager::getPost() {
    return PostsVec;
}

Post* PostsManager::findPost(int postId) {
    for (auto& post : PostsVec) {
        if (post.getPostId() == postId) {
            return &post;
        }
    }
    return nullptr;
}

void Timeline::addReaction(int postId, const string& username, const string& reaction) {
    Post* post = findPost(postId);
    if (!post) {
        throw runtime_error("Post not found");
    }
    
    // Toggle reaction - if user already reacted, remove it; otherwise add it
    if (post->hasReaction(username)) {
        post->removeReaction(username);
    } else {
        post->addReaction(username);
    }
    
    // Save changes to file
    savePosts();
}

// Add this new method to the Timeline class
vector<Post> Timeline::getFilteredPosts(const string& username, const FriendsManager& friendsManager) {
    vector<Post> filteredPosts;
    for (const auto& post : PostsVec) {
        // Include posts if they are from the user or from their friends
        if (post.getPostOwner() == username || friendsManager.areFriends(username, post.getPostOwner())) {
            filteredPosts.push_back(post);
        }
    }
    // Sort by timestamp, newest first
    sort(filteredPosts.begin(), filteredPosts.end(), 
        [](const Post& a, const Post& b) {
            return a.getPostTimes() > b.getPostTimes();
        });
    return filteredPosts;
}

