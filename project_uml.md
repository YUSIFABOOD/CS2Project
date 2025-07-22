# Social Media Platform UML Documentation

## Class Diagram

```mermaid
classDiagram
    class Authentication {
        -unordered_map~string, User~ usersByUsername
        -unordered_map~string, string~ sessions
        -unordered_map~string, string~ usersnameToToken
        -string db_path_
        -string sessions_path_
        +Authentication(string db_path)
        +login(string name, string pass) string
        +signup(string name, string pass) void
        +logout(string token) void
        +getUserByToken(string token) User*
        +isLoggedIn(string token) bool
        +verifyToken(string token) string
        +userExists(string name) bool
        +getUsers() unordered_map~string, User~&
        +loadSessions() void
        +saveSessions() void
        +hashPass(string pass, string salt) string$
        +generateSalt() string$
        +generateSession() string$
    }

    class User {
        -string username
        -string hashedPass
        -string salt
        -AVLTree~string~ friends
        +User(string name, string pass, string salt)
        +getUsername() string
        +getHashedPass() string
        +getSalt() string
        +getFriends() AVLTree~string~&
        +toJson() json
        +fromJson(json j) User$
    }

    class AVLTree~T~ {
        -Node* root
        +insert(T value) void
        +remove(T value) void
        +contains(T value) bool
        +inOrder() vector~T~
        -balance(Node* node) Node*
        -rotateLeft(Node* node) Node*
        -rotateRight(Node* node) Node*
        -getHeight(Node* node) int
    }

    class FriendsManager {
        -unordered_map~string, User~& users
        -unordered_map~string, unordered_set~string~~ pendingRequests
        +FriendsManager(unordered_map~string, User~& usersMap)
        +sendFriendRequest(string from, string to) bool
        +acceptFriendRequest(string from, string to) bool
        +rejectFriendRequest(string from, string to) bool
        +removeFriend(string username, string friendName) bool
        +getFriendList(string username) vector~string~
        +getPendingRequests(string username) vector~string~
        +areFriends(string userA, string userB) bool
        +getMutualFriends(string userA, string userB) vector~string~
        +suggestFriends(string username) vector~string~
        +saveFriends(string filename) void
        +loadFriends(string filename) void
        +savePendingRequests(string filename) void
        +loadPendingRequests(string filename) void
        +getFriendCount(string username) int
    }

    class UserSearchBST {
        -Node* root
        +insertUser(string username) void
        +removeUser(string username) void
        +searchByPrefix(string prefix) vector~string~
        +searchBySubstring(string substring) vector~string~
        -searchByPrefix(Node* node, string prefix, vector~string~& results) void
        -searchBySubstring(Node* node, string substring, vector~string~& results) void
    }

    class Post {
        -int id
        -string content
        -string owner
        -time_t timestamp
        -vector~Comment~ commentVec
        -vector~string~ reactions
        -int nextCommentId
        +Post(int id, string content, string owner)
        +getPostId() int
        +getPostContent() string
        +getPostOwner() string
        +getPostTimes() time_t
        +AddComment(string comment, string username) void
        +EditComment(string newComment, string username, int commentId) void
        +deleteComment(string username, int commentId) void
        +getComments() vector~Comment~&
        +addReaction(string username) void
        +removeReaction(string username) void
        +hasReaction(string username) bool
        +getReactionCount() int
        +getReactions() vector~string~&
        +Edit(string newContent) void
        +PostToJson() json
        +fromJson(json j) Post$
    }

    class Comment {
        -int commentId
        -int postId
        -string owner
        -string content
        -time_t timestamp
        +Comment(int id, int pId, string owner, string content)
        +getCommentId() int
        +getCommentContent() string
        +getCommentOwner() string
        +getCommentTimes() time_t
        +setContent(string content) void
        +setTimestamp(time_t newTime) void
        +CommentToJson() json
        +CommentFromJson(json j) Comment$
    }

    class Timeline {
        -vector~Post~ PostsVec
        -string filePath
        -int nextPostId
        +Timeline(string file)
        +Add_post(string post, string name) void
        +loadPosts() void
        +savePosts() void
        +EditPost(int id, string username, string newContent) void
        +deletePost(int id) void
        +getPost() vector~Post~&
        +findPost(int postId) Post*
        +addReaction(int postId, string username, string reaction) void
        +getFilteredPosts(string username, FriendsManager friendsManager) vector~Post~
    }

    Authentication "1" --> "*" User : manages
    User "1" --> "1" AVLTree : has friends
    FriendsManager "1" --> "*" User : manages
    Timeline "1" --> "*" Post : contains
    Post "1" --> "*" Comment : has
    UserSearchBST "1" --> "*" User : indexes
    Timeline --|> PostsManager : extends
    Post ..> Comment : creates
    FriendsManager ..> User : references
    Authentication ..> User : creates
```

## Component Descriptions

### 1. Core User Management
- **Authentication**: Central class for user management and security
- **User**: Represents user entities with their data and friend relationships
- **AVLTree**: Generic balanced tree used for efficient friend list storage

### 2. Social Features
- **FriendsManager**: Handles all friendship-related operations
- **UserSearchBST**: Provides efficient user search functionality
- **Timeline**: Manages the social feed and post organization

### 3. Content Management
- **Post**: Represents social media posts with reactions and comments
- **Comment**: Represents comments on posts
- **Timeline**: Manages post collections and filtering

### 4. Key Relationships
- Authentication manages Users (1-to-many)
- Each User has an AVLTree for friends (1-to-1)
- FriendsManager manages User relationships (1-to-many)
- Timeline contains Posts (1-to-many)
- Posts contain Comments (1-to-many)
- UserSearchBST indexes Users (1-to-many)

### 5. Data Structures
- AVL Tree: For balanced friend list storage
- Binary Search Tree: For efficient user search
- Hash Maps: For user storage and session management
- Vectors: For posts and comments storage

### 6. Features
- Token-based authentication
- Friend request system
- Reaction system
- Comment management
- Post filtering based on friendships
- Efficient user search with prefix and substring matching

### 7. Persistence
- JSON serialization for all data types
- File-based storage for users, posts, friends, and sessions
- Separate files for pending friend requests

## Architecture Benefits
- Efficient data access and manipulation
- Clear separation of concerns
- Scalable social features
- Secure user management
- Persistent data storage
- Optimized search capabilities 