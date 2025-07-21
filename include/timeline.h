#include <bits/stdc++.h>
#include <crow.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <ctime>
#include <filesystem>
using namespace std;

namespace fs = std::filesystem;
using json = nlohmann::json;

//-----------------------------------------------------------------
//Post and comment classes
//-----------------------------------------------------------------
class Comment {
    int commentId;
    int postId;
    string owner;
    string content;
    time_t timestamp;
public:
    Comment(int id, int pId, const string& o, const string& c);
    int getCommentId() const;
    void setContent(const string& c);
    void setTimestamp(time_t newTime);
    string getCommentContent() const;
    string getCommentOwner() const;
    time_t getCommentTimes() const;
    int getPostId() const { return postId; }
    json CommentToJson() const;
    static Comment CommentFromJson(const json& Json);
};


class Post{
private:
    int id;
    string content;
    string owner;
    time_t timestamp;
    vector <Comment> commentVec;
    int nextCommentId=1;
    vector<string> reactions; // Store usernames who liked the post
public:
    Post(int id, const string& content, const string& owner);

    int getPostId() const;
    string getPostContent() const;
    string getPostOwner() const;
    time_t getPostTimes() const;
    json PostToJson() const;
    static Post fromJson(const json& j);
    void Edit(const string& newContent);
//------------------------------------------------------------
    //funcs to manage comments
    void setNextCommentId(int nextId) { nextCommentId = nextId; }
    void AddComment(const string& comment, const string& username);
        void EditComment(const string& newComment, const string& username, int commentId);
        void deleteComment(const string& username, int commentId);
    const vector <Comment>& getComments () const;
    int getNextCommentId() const { return nextCommentId; }
    //--------------------------------------
    // funcs to manage reactions
    void addReaction(const string& username);
    void removeReaction(const string& username);
    bool hasReaction(const string& username) const;
    int getReactionCount() const;
    const vector<string>& getReactions() const;
    //--------------------------------------

};

//-----------------------------------------------------------------
// Posts manager class
//-----------------------------------------------------------------

class PostsManager {
protected:
    string filePath;
    vector<Post> PostsVec;
    int nextPostId = 1;
    json posts_data;
public:
    PostsManager(const string& file); 
    void loadPosts();
    void savePosts(); 
    //--------------------------------------
    //funcs to manage posts
    void Add_post(const string& post, const string& username);
    void EditPost(int id, string& username, const string& newContent);
    void deletePost(int id);
    vector<Post>& getPost();
    Post* findPost(int postId);
    int getNextPostId() const { return nextPostId; }
    void setNextPostId(int nextId) { nextPostId = nextId; }
    //--------------------------------------
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class Timeline : public PostsManager {
public:
    Timeline(const string& file = "../database/posts.json") : PostsManager(file) {}
    void sortByTime();
    void showComments(int postId) const;
    void addReaction(int postId, const string& username, const string& reaction);
};