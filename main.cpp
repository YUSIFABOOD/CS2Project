#include "include/Authentication.h"
#include "include/Users.h"
#include "include/timeline.h"
#include "include/FriendsManager.h"
#include "include/UserSearchBST.h"
#include <crow.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <libgen.h>
#include <algorithm>

namespace fs = std::filesystem;

// Helper function to add CORS headers
void add_cors_headers(crow::response& res, const crow::request& req) {
    // Get the Origin header from the request
    std::string origin = req.get_header_value("Origin");
    if (origin.empty()) {
        // If no Origin header, allow all origins
        res.set_header("Access-Control-Allow-Origin", "*");
    } else {
        // Otherwise, echo back the Origin header
        res.set_header("Access-Control-Allow-Origin", origin);
    }
    
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, Accept");
    res.set_header("Access-Control-Allow-Credentials", "true");
    res.set_header("Access-Control-Max-Age", "3600");
    res.set_header("Vary", "Origin");
}

// Helper function to read file content
std::string readFile(const fs::path& project_root, const std::string& relative_path) {
    fs::path file_path = project_root / relative_path;
    std::cout << "Reading file: " << file_path << std::endl;

    if (fs::exists(file_path)) {
        std::ifstream file(file_path);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }
    }

    std::cerr << "Failed to open file: " << file_path << std::endl;
    return "";
}

// Helper function to create JSON response
crow::response makeJsonResponse(const crow::request& req, int code, const std::string& message, bool isError = false) {
    crow::json::wvalue json;
    if (isError) {
        json["error"] = message;
    } else {
        json["message"] = message;
    }
    
    auto res = crow::response(code);
    add_cors_headers(res, req);
    res.set_header("Content-Type", "application/json");
    res.body = json.dump();
    return res;
}

// Helper function to find the project root by looking for a marker file (e.g., CMakeLists.txt)
fs::path find_project_root(fs::path start_path) {
    fs::path current_path = fs::absolute(start_path);
    while (!current_path.empty() && current_path != current_path.root_path()) {
        if (fs::exists(current_path / "CMakeLists.txt")) {
            return current_path;
        }
        current_path = current_path.parent_path();
    }
    return ""; // Return empty path if not found
}

// Helper function to extract token from Authorization header
std::string getTokenFromRequest(const crow::request& req) {
    std::string authHeader = req.get_header_value("Authorization");
    if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
        throw std::runtime_error("No valid token provided");
    }
    return authHeader.substr(7);
}

int main(int argc, char* argv[]) {
    srand(time(0));
    crow::SimpleApp app;

    // Determine the executable's path to locate the database directory
    fs::path executable_path(argv[0]);
    fs::path project_root = find_project_root(executable_path.parent_path());
    if (project_root.empty()) {
        std::cerr << "Error: Could not find project root. Make sure CMakeLists.txt is present." << std::endl;
        return 1;
    }
    std::cout << "Executable path: " << fs::absolute(executable_path) << std::endl;
    std::cout << "Project root: " << project_root << std::endl;
    fs::path db_path = project_root / "database";
    fs::path users_db_path = db_path / "users.json";
    fs::path posts_db_path = db_path / "posts.json";
    fs::path friends_db_path = db_path / "friends.json";
    fs::path pending_requests_db_path = db_path / "pending_requests.json";

    // Initialize Authentication
    std::unique_ptr<Authentication> auth;
    try {
        auth = std::make_unique<Authentication>(users_db_path.string());
        std::cout << "Authentication system initialized successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize authentication system: " << e.what() << std::endl;
        return 1;
    }

    // Initialize Timeline
    Timeline timeline(posts_db_path.string());  // Initialize Timeline directly with file path

    // Initialize FriendsManager
    std::unique_ptr<FriendsManager> friendsManager;
    try {
        friendsManager = std::make_unique<FriendsManager>(auth->getUsers());
        friendsManager->loadFriends(friends_db_path.string());
        friendsManager->loadPendingRequests(pending_requests_db_path.string());
        std::cout << "Friends management system initialized successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize friends management system: " << e.what() << std::endl;
        return 1;
    }

    // Initialize User Search BST
    std::unique_ptr<UserSearchBST> userSearchBST;
    try {
        userSearchBST = std::make_unique<UserSearchBST>();
        // Populate BST with all users
        const auto& users = auth->getUsers();
        std::vector<std::string> usernames;
        for (const auto& pair : users) {
            usernames.push_back(pair.first);
        }
        userSearchBST->rebuildFromUsers(usernames);
        std::cout << "User search BST initialized with " << usernames.size() << " users" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize user search BST: " << e.what() << std::endl;
        return 1;
    }

    // Handle OPTIONS requests for CORS
    CROW_ROUTE(app, "/<path>").methods("OPTIONS"_method)([](const crow::request& req, std::string) {
        auto res = crow::response(204);
        add_cors_headers(res, req);
        return res;
    });

    // --------- ROOT ROUTE ----------
    CROW_ROUTE(app, "/")([&project_root]() {
        std::string content = readFile(project_root, "assets/index.html");
        if (content.empty()) {
            return crow::response(404, "Frontend not found");
        }
        auto res = crow::response(content);
        res.set_header("Content-Type", "text/html");
        return res;
    });

    // --------- SIGNUP ----------
    CROW_ROUTE(app, "/api/auth/signup").methods("POST"_method)([&auth](const crow::request& req) {
        std::cout << "\n=== Processing Signup Request ===" << std::endl;
        std::cout << "Request body: " << req.body << std::endl;

        try {
            auto data = crow::json::load(req.body);
            if (!data) {
                return makeJsonResponse(req, 400, "Invalid JSON format", true);
            }

            if (!data.has("username") || !data.has("password")) {
                return makeJsonResponse(req, 400, "Missing username or password", true);
            }

            std::string username = data["username"].s();
            std::string password = data["password"].s();

            auth->signup(username, password);
            
            // Generate token after successful signup
            std::string token = auth->login(username, password);
            
            crow::json::wvalue result;
            result["success"] = true;
            result["token"] = token;
            result["username"] = username;
            result["message"] = "Signup successful";

            auto res = crow::response(200);
            add_cors_headers(res, req);
            res.set_header("Content-Type", "application/json");
            res.body = result.dump();
            return res;

        } catch (const std::exception& e) {
            std::cerr << "Signup error: " << e.what() << std::endl;
            return makeJsonResponse(req, 400, e.what(), true);
        }
    });

    // --------- LOGIN ----------
    CROW_ROUTE(app, "/api/auth/login").methods("POST"_method)([&auth](const crow::request& req) {
        std::cout << "\n=== Processing Login Request ===" << std::endl;
        std::cout << "Request body: " << req.body << std::endl;

        try {
            auto data = crow::json::load(req.body);
            if (!data) {
                return makeJsonResponse(req, 400, "Invalid JSON format", true);
            }

            if (!data.has("username") || !data.has("password")) {
                return makeJsonResponse(req, 400, "Missing username or password", true);
            }

            std::string username = data["username"].s();
            std::string password = data["password"].s();

            std::string token = auth->login(username, password);
            if (token.empty()) {
                crow::json::wvalue result;
                result["success"] = false;
                result["message"] = "Invalid username or password";
                
                auto res = crow::response(401);
                add_cors_headers(res, req);
                res.set_header("Content-Type", "application/json");
                res.body = result.dump();
                return res;
            }

            crow::json::wvalue result;
            result["success"] = true;
            result["token"] = token;
            result["username"] = username;
            result["message"] = "Login successful";

            auto res = crow::response(200);
            add_cors_headers(res, req);
            res.set_header("Content-Type", "application/json");
            res.body = result.dump();
            return res;

        } catch (const std::exception& e) {
            std::cerr << "Login error: " << e.what() << std::endl;
            return makeJsonResponse(req, 400, e.what(), true);
        }
    });

    // --------- VERIFY TOKEN ----------
    CROW_ROUTE(app, "/api/auth/verify").methods("GET"_method)([&auth](const crow::request& req) {
        std::string authHeader = req.get_header_value("Authorization");
        if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
            crow::json::wvalue result;
            result["success"] = false;
            result["message"] = "No token provided";
            
            auto res = crow::response(401);
            add_cors_headers(res, req);
            res.set_header("Content-Type", "application/json");
            res.body = result.dump();
            return res;
        }

        std::string token = authHeader.substr(7);
        try {
            std::string username = auth->verifyToken(token);
            
            crow::json::wvalue result;
            result["success"] = true;
            result["username"] = username;
            result["user"] = username;  // Frontend expects 'user' field
            
            auto res = crow::response(200);
            add_cors_headers(res, req);
            res.set_header("Content-Type", "application/json");
            res.body = result.dump();
            return res;
        } catch (const std::exception& e) {
            crow::json::wvalue result;
            result["success"] = false;
            result["message"] = e.what();
            
            auto res = crow::response(401);
            add_cors_headers(res, req);
            res.set_header("Content-Type", "application/json");
            res.body = result.dump();
            return res;
        }
    });

    // --------- POSTS ----------
    // Get all posts
    CROW_ROUTE(app, "/api/posts").methods("GET"_method)([&timeline](const crow::request& req) {
        try {
            crow::json::wvalue result = crow::json::wvalue::list();
            vector<Post>& posts = timeline.getPost();

            for (const auto& post : posts) {
                crow::json::wvalue post_json;
                post_json["id"] = post.getPostId();
                post_json["content"] = post.getPostContent();
                post_json["owner"] = post.getPostOwner();
                post_json["timestamp"] = post.getPostTimes();

                // Add reactions
                crow::json::wvalue reactions_json = crow::json::wvalue::list();
                const auto& reactions = post.getReactions();
                int reaction_idx = 0;
                for (const auto& reaction : reactions) {
                    reactions_json[reaction_idx++] = reaction;
                }
                post_json["reactions"] = std::move(reactions_json);

                // Add comments
                crow::json::wvalue comments_json = crow::json::wvalue::list();
                const auto& comments = post.getComments();
                int comment_idx = 0;
                for (const auto& comment : comments) {
                    crow::json::wvalue comment_json;
                    comment_json["id"] = comment.getCommentId();
                    comment_json["content"] = comment.getCommentContent();
                    comment_json["owner"] = comment.getCommentOwner();
                    comment_json["timestamp"] = comment.getCommentTimes();
                    comments_json[comment_idx++] = std::move(comment_json);
                }
                post_json["comments"] = std::move(comments_json);
                result[result.size()] = std::move(post_json);
            }

            auto res = crow::response(200);
            add_cors_headers(res, req);
            res.set_header("Content-Type", "application/json");
            res.body = result.dump();
            return res;
        } catch (const std::exception& e) {
            return makeJsonResponse(req, 500, e.what(), true);
        }
    });

    // Get friends-only posts
    CROW_ROUTE(app, "/api/posts/friends").methods("GET"_method)([&timeline, &auth, &friendsManager](const crow::request& req) {
        try {
            std::string token = getTokenFromRequest(req);
            if (token.empty()) {
                return makeJsonResponse(req, 401, "Authentication required", true);
            }

            std::string currentUser = auth->verifyToken(token);
            if (currentUser.empty()) {
                return makeJsonResponse(req, 401, "Invalid token", true);
            }

            // Get user's friends list
            std::vector<std::string> friends = friendsManager->getFriendList(currentUser);
            
            // Add current user to see their own posts too
            friends.push_back(currentUser);

            crow::json::wvalue result = crow::json::wvalue::list();
            vector<Post>& posts = timeline.getPost();

            for (const auto& post : posts) {
                // Check if post owner is in friends list
                bool isFriend = std::find(friends.begin(), friends.end(), post.getPostOwner()) != friends.end();
                
                if (isFriend) {
                    crow::json::wvalue post_json;
                    post_json["id"] = post.getPostId();
                    post_json["content"] = post.getPostContent();
                    post_json["owner"] = post.getPostOwner();
                    post_json["timestamp"] = post.getPostTimes();

                    // Add reactions
                    crow::json::wvalue reactions_json = crow::json::wvalue::list();
                    const auto& reactions = post.getReactions();
                    int reaction_idx = 0;
                    for (const auto& reaction : reactions) {
                        reactions_json[reaction_idx++] = reaction;
                    }
                    post_json["reactions"] = std::move(reactions_json);

                    // Add comments
                    crow::json::wvalue comments_json = crow::json::wvalue::list();
                    const auto& comments = post.getComments();
                    int comment_idx = 0;
                    for (const auto& comment : comments) {
                        crow::json::wvalue comment_json;
                        comment_json["id"] = comment.getCommentId();
                        comment_json["content"] = comment.getCommentContent();
                        comment_json["owner"] = comment.getCommentOwner();
                        comment_json["timestamp"] = comment.getCommentTimes();
                        comments_json[comment_idx++] = std::move(comment_json);
                    }
                    post_json["comments"] = std::move(comments_json);
                    result[result.size()] = std::move(post_json);
                }
            }

            auto res = crow::response(200);
            add_cors_headers(res, req);
            res.set_header("Content-Type", "application/json");
            res.body = result.dump();
            return res;
        } catch (const std::exception& e) {
            return makeJsonResponse(req, 500, e.what(), true);
        }
    });

    // Create new post
    CROW_ROUTE(app, "/api/posts/create").methods("POST"_method)([&timeline, &auth](const crow::request& req) {
        // Verify token
        std::string authHeader = req.get_header_value("Authorization");
        if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
            return makeJsonResponse(req, 401, "Unauthorized", true);
        }

        try {
            std::string username = auth->verifyToken(authHeader.substr(7));
            auto data = crow::json::load(req.body);
            if (!data || !data.has("content")) {
                return makeJsonResponse(req, 400, "Invalid post data", true);
            }

            timeline.Add_post(data["content"].s(), username);
            
            crow::json::wvalue result;
            result["success"] = true;
            result["message"] = "Post created successfully";
            
            auto res = crow::response(201);
            add_cors_headers(res, req);
            res.set_header("Content-Type", "application/json");
            res.body = result.dump();
            return res;
        } catch (const std::exception& e) {
            return makeJsonResponse(req, 500, e.what(), true);
        }
    });

    // Delete post
    CROW_ROUTE(app, "/api/posts/<string>").methods("DELETE"_method)([&timeline, &auth](const crow::request& req, std::string postId) {
        // Verify token
        std::string authHeader = req.get_header_value("Authorization");
        if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
            return makeJsonResponse(req, 401, "Unauthorized", true);
        }

        try {
            std::string username = auth->verifyToken(authHeader.substr(7));
            timeline.deletePost(std::stoi(postId));
            
            crow::json::wvalue result;
            result["success"] = true;
            result["message"] = "Post deleted successfully";
            
            auto res = crow::response(200);
            add_cors_headers(res, req);
            res.set_header("Content-Type", "application/json");
            res.body = result.dump();
            return res;
        } catch (const std::exception& e) {
            return makeJsonResponse(req, 500, e.what(), true);
        }
    });

    // Edit post
    CROW_ROUTE(app, "/api/posts/<string>").methods("PUT"_method)([&timeline, &auth](const crow::request& req, std::string postId) {
        // Verify token
        std::string authHeader = req.get_header_value("Authorization");
        if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
            return makeJsonResponse(req, 401, "Unauthorized", true);
        }

        try {
            std::string username = auth->verifyToken(authHeader.substr(7));
            auto data = crow::json::load(req.body);
            if (!data || !data.has("content")) {
                return makeJsonResponse(req, 400, "Invalid post data", true);
            }

            timeline.EditPost(std::stoi(postId), username, data["content"].s());
            
            crow::json::wvalue result;
            result["success"] = true;
            result["message"] = "Post updated successfully";
            
            auto res = crow::response(200);
            add_cors_headers(res, req);
            res.set_header("Content-Type", "application/json");
            res.body = result.dump();
            return res;
        } catch (const std::exception& e) {
            return makeJsonResponse(req, 500, e.what(), true);
        }
    });

    // Add comment
    CROW_ROUTE(app, "/api/posts/<string>/comment").methods("POST"_method)([&timeline, &auth](const crow::request& req, std::string postId) {
        // Verify token
        std::string authHeader = req.get_header_value("Authorization");
        if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
            return makeJsonResponse(req, 401, "Unauthorized", true);
        }

        try {
            std::string username = auth->verifyToken(authHeader.substr(7));
            auto data = crow::json::load(req.body);
            if (!data || !data.has("content")) {
                return makeJsonResponse(req, 400, "Invalid comment data", true);
            }

            Post* post = timeline.findPost(std::stoi(postId));
            if (!post) {
                return makeJsonResponse(req, 404, "Post not found", true);
            }

            post->AddComment(data["content"].s(), username);
            timeline.savePosts();  // Save after adding comment
            
            crow::json::wvalue result;
            result["success"] = true;
            result["message"] = "Comment added successfully";
            
            auto res = crow::response(201);
            add_cors_headers(res, req);
            res.set_header("Content-Type", "application/json");
            res.body = result.dump();
            return res;
        } catch (const std::exception& e) {
            return makeJsonResponse(req, 500, e.what(), true);
        }
    });

    // Add reaction
    CROW_ROUTE(app, "/api/posts/<string>/react").methods("POST"_method)([&timeline, &auth](const crow::request& req, std::string postId) {
        // Verify token
        std::string authHeader = req.get_header_value("Authorization");
        if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
            return makeJsonResponse(req, 401, "Unauthorized", true);
        }

        try {
            std::string username = auth->verifyToken(authHeader.substr(7));
            std::cout << "Reaction request from user: " << username << " for post: " << postId << std::endl;
            
            auto data = crow::json::load(req.body);
            if (!data || !data.has("type")) {
                return makeJsonResponse(req, 400, "Invalid reaction data", true);
            }

            std::cout << "Calling timeline.addReaction..." << std::endl;
            timeline.addReaction(std::stoi(postId), username, data["type"].s());
            std::cout << "Reaction added successfully" << std::endl;
            
            crow::json::wvalue result;
            result["success"] = true;
            result["message"] = "Reaction added successfully";
            
            auto res = crow::response(200);
            add_cors_headers(res, req);
            res.set_header("Content-Type", "application/json");
            res.body = result.dump();
            return res;
        } catch (const std::exception& e) {
            std::cout << "Error in reaction endpoint: " << e.what() << std::endl;
            return makeJsonResponse(req, 500, e.what(), true);
        }
    });

    // Edit a comment
    CROW_ROUTE(app, "/api/posts/<string>/comment/<string>").methods("PUT"_method)([&timeline, &auth](const crow::request& req, std::string postId, std::string commentId) {
        std::string authHeader = req.get_header_value("Authorization");
        if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
            return makeJsonResponse(req, 401, "Unauthorized", true);
        }

        try {
            std::string username = auth->verifyToken(authHeader.substr(7));
            auto data = crow::json::load(req.body);
            if (!data || !data.has("content")) {
                return makeJsonResponse(req, 400, "Invalid comment data", true);
            }

            Post* post = timeline.findPost(std::stoi(postId));
            if (!post) {
                return makeJsonResponse(req, 404, "Post not found", true);
            }

            post->EditComment(data["content"].s(), username, std::stoi(commentId));
            timeline.savePosts();

            return makeJsonResponse(req, 200, "Comment updated successfully");
        } catch (const std::exception& e) {
            return makeJsonResponse(req, 500, e.what(), true);
        }
    });

    // Delete a comment
    CROW_ROUTE(app, "/api/posts/<string>/comment/<string>").methods("DELETE"_method)([&timeline, &auth](const crow::request& req, std::string postId, std::string commentId) {
        std::string authHeader = req.get_header_value("Authorization");
        if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
            return makeJsonResponse(req, 401, "Unauthorized", true);
        }

        try {
            std::string username = auth->verifyToken(authHeader.substr(7));
            Post* post = timeline.findPost(std::stoi(postId));
            if (!post) {
                return makeJsonResponse(req, 404, "Post not found", true);
            }

            post->deleteComment(username, std::stoi(commentId));
            timeline.savePosts();

            return makeJsonResponse(req, 200, "Comment deleted successfully");
        } catch (const std::exception& e) {
            return makeJsonResponse(req, 500, e.what(), true);
        }
    });

    // --------- FRIENDSHIP MANAGEMENT ENDPOINTS ----------
    
    // Send friend request
    CROW_ROUTE(app, "/api/friends/request").methods("POST"_method)([&](const crow::request& req) {
        try {
            std::string username = auth->verifyToken(getTokenFromRequest(req));
            
            auto data = crow::json::load(req.body);
            if (!data) {
                return makeJsonResponse(req, 400, "Invalid JSON format", true);
            }
            
            if (!data.has("to")) {
                return makeJsonResponse(req, 400, "Missing 'to' field", true);
            }
            
            std::string to = data["to"].s();
            
            if (username == to) {
                return makeJsonResponse(req, 400, "Cannot send friend request to yourself", true);
            }
            
            if (!auth->userExists(to)) {
                return makeJsonResponse(req, 404, "User not found", true);
            }
            
            bool success = friendsManager->sendFriendRequest(username, to);
            if (!success) {
                return makeJsonResponse(req, 400, "Friend request already sent or users are already friends", true);
            }
            
            friendsManager->savePendingRequests(pending_requests_db_path.string());
            
            crow::json::wvalue result;
            result["success"] = true;
            result["message"] = "Friend request sent successfully";
            
            auto res = crow::response(200);
            add_cors_headers(res, req);
            res.set_header("Content-Type", "application/json");
            res.body = result.dump();
            return res;
            
        } catch (const std::exception& e) {
            return makeJsonResponse(req, 500, e.what(), true);
        }
    });
    
    // Accept friend request
    CROW_ROUTE(app, "/api/friends/accept").methods("POST"_method)([&](const crow::request& req) {
        try {
            std::string username = auth->verifyToken(getTokenFromRequest(req));
            
            auto data = crow::json::load(req.body);
            if (!data) {
                return makeJsonResponse(req, 400, "Invalid JSON format", true);
            }
            
            if (!data.has("from")) {
                return makeJsonResponse(req, 400, "Missing 'from' field", true);
            }
            
            std::string from = data["from"].s();
            
            bool success = friendsManager->acceptFriendRequest(from, username);
            if (!success) {
                return makeJsonResponse(req, 400, "No pending friend request from this user", true);
            }
            
            friendsManager->saveFriends(friends_db_path.string());
            friendsManager->savePendingRequests(pending_requests_db_path.string());
            
            crow::json::wvalue result;
            result["success"] = true;
            result["message"] = "Friend request accepted";
            
            auto res = crow::response(200);
            add_cors_headers(res, req);
            res.set_header("Content-Type", "application/json");
            res.body = result.dump();
            return res;
            
        } catch (const std::exception& e) {
            return makeJsonResponse(req, 500, e.what(), true);
        }
    });
    
    // Reject friend request
    CROW_ROUTE(app, "/api/friends/reject").methods("POST"_method)([&](const crow::request& req) {
        try {
            std::string username = auth->verifyToken(getTokenFromRequest(req));
            
            auto data = crow::json::load(req.body);
            if (!data) {
                return makeJsonResponse(req, 400, "Invalid JSON format", true);
            }
            
            if (!data.has("from")) {
                return makeJsonResponse(req, 400, "Missing 'from' field", true);
            }
            
            std::string from = data["from"].s();
            
            bool success = friendsManager->rejectFriendRequest(from, username);
            if (!success) {
                return makeJsonResponse(req, 400, "No pending friend request from this user", true);
            }
            
            friendsManager->savePendingRequests(pending_requests_db_path.string());
            
            crow::json::wvalue result;
            result["success"] = true;
            result["message"] = "Friend request rejected";
            
            auto res = crow::response(200);
            add_cors_headers(res, req);
            res.set_header("Content-Type", "application/json");
            res.body = result.dump();
            return res;
            
        } catch (const std::exception& e) {
            return makeJsonResponse(req, 500, e.what(), true);
        }
    });
    
    // Remove friend
    CROW_ROUTE(app, "/api/friends/remove").methods("POST"_method)([&](const crow::request& req) {
        try {
            std::string username = auth->verifyToken(getTokenFromRequest(req));
            
            auto data = crow::json::load(req.body);
            if (!data) {
                return makeJsonResponse(req, 400, "Invalid JSON format", true);
            }
            
            if (!data.has("friend")) {
                return makeJsonResponse(req, 400, "Missing 'friend' field", true);
            }
            
            std::string friendName = data["friend"].s();
            
            bool success = friendsManager->removeFriend(username, friendName);
            if (!success) {
                return makeJsonResponse(req, 400, "Users are not friends", true);
            }
            
            friendsManager->saveFriends(friends_db_path.string());
            
            crow::json::wvalue result;
            result["success"] = true;
            result["message"] = "Friend removed successfully";
            
            auto res = crow::response(200);
            add_cors_headers(res, req);
            res.set_header("Content-Type", "application/json");
            res.body = result.dump();
            return res;
            
        } catch (const std::exception& e) {
            return makeJsonResponse(req, 500, e.what(), true);
        }
    });
    
    // Get friends list
    CROW_ROUTE(app, "/api/friends").methods("GET"_method)([&](const crow::request& req) {
        try {
            std::string username = auth->verifyToken(getTokenFromRequest(req));
            
            std::vector<std::string> friends = friendsManager->getFriendList(username);
            
            crow::json::wvalue result;
            result["success"] = true;
            result["friends"] = crow::json::wvalue::list();
            
            for (size_t i = 0; i < friends.size(); i++) {
                result["friends"][i] = friends[i];
            }
            
            auto res = crow::response(200);
            add_cors_headers(res, req);
            res.set_header("Content-Type", "application/json");
            res.body = result.dump();
            return res;
            
        } catch (const std::exception& e) {
            return makeJsonResponse(req, 500, e.what(), true);
        }
    });
    
    // Get pending friend requests
    CROW_ROUTE(app, "/api/friends/pending").methods("GET"_method)([&](const crow::request& req) {
        try {
            std::string username = auth->verifyToken(getTokenFromRequest(req));
            
            std::vector<std::string> pending = friendsManager->getPendingRequests(username);
            
            crow::json::wvalue result;
            result["success"] = true;
            result["pending_requests"] = crow::json::wvalue::list();
            
            for (size_t i = 0; i < pending.size(); i++) {
                result["pending_requests"][i] = pending[i];
            }
            
            auto res = crow::response(200);
            add_cors_headers(res, req);
            res.set_header("Content-Type", "application/json");
            res.body = result.dump();
            return res;
            
        } catch (const std::exception& e) {
            return makeJsonResponse(req, 500, e.what(), true);
        }
    });
    
    // Get friend suggestions
    CROW_ROUTE(app, "/api/friends/suggestions").methods("GET"_method)([&](const crow::request& req) {
        try {
            std::string username = auth->verifyToken(getTokenFromRequest(req));
            
            std::vector<std::string> suggestions = friendsManager->suggestFriends(username);
            
            crow::json::wvalue result;
            result["success"] = true;
            result["suggestions"] = crow::json::wvalue::list();
            
            for (size_t i = 0; i < suggestions.size(); i++) {
                result["suggestions"][i] = suggestions[i];
            }
            
            auto res = crow::response(200);
            add_cors_headers(res, req);
            res.set_header("Content-Type", "application/json");
            res.body = result.dump();
            return res;
            
        } catch (const std::exception& e) {
            return makeJsonResponse(req, 500, e.what(), true);
        }
    });

    // User search endpoint using BST
    CROW_ROUTE(app, "/api/users/search").methods("GET"_method)([&auth, &userSearchBST](const crow::request& req) {
        try {
            std::cout << "\n=== SEARCH REQUEST RECEIVED ===" << std::endl;
            std::string token = getTokenFromRequest(req);
            if (token.empty()) {
                std::cout << "No token provided" << std::endl;
                return makeJsonResponse(req, 401, "Authentication required", true);
            }

            std::string currentUser = auth->verifyToken(token);
            if (currentUser.empty()) {
                std::cout << "Invalid token" << std::endl;
                return makeJsonResponse(req, 401, "Invalid token", true);
            }
            std::cout << "Search request from user: " << currentUser << std::endl;

            // Get search query and type from URL parameters
            std::string query = req.url_params.get("q") ? req.url_params.get("q") : "";
            std::string searchType = req.url_params.get("type") ? req.url_params.get("type") : "substring";
            std::cout << "Search query: '" << query << "', type: " << searchType << std::endl;
            
            if (query.empty()) {
                std::cout << "Empty query provided" << std::endl;
                return makeJsonResponse(req, 400, "Search query is required", true);
            }

            json response;
            response["success"] = true;
            response["users"] = json::array();
            response["search_type"] = searchType;

            // Use BST for efficient searching
            std::vector<std::string> searchResults;
            if (searchType == "prefix") {
                std::cout << "Performing prefix search..." << std::endl;
                searchResults = userSearchBST->searchByPrefix(query);
            } else {
                std::cout << "Performing substring search..." << std::endl;
                searchResults = userSearchBST->searchBySubstring(query);
            }
            
            std::cout << "BST search returned " << searchResults.size() << " results:";
            for (const auto& result : searchResults) {
                std::cout << " '" << result << "'";
            }
            std::cout << std::endl;

            // Filter out current user and add to response
            for (const auto& username : searchResults) {
                if (username != currentUser) {
                    response["users"].push_back(username);
                    std::cout << "Added to response: " << username << std::endl;
                }
            }

            return makeJsonResponse(req, 200, response.dump());
        } catch (const std::exception& e) {
            return makeJsonResponse(req, 500, e.what(), true);
        }
    });

    app.port(18080).multithreaded().run();

    return 0;
}
