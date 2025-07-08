#include "include/Authentication.h"
#include "include/Users.h"
#include <crow.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <libgen.h>

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
std::string readFile(const std::string& relative_path) {
    // First try relative to build directory
    std::string path = "../" + relative_path;
    std::cout << "Reading file: " << path << std::endl;
    
    if (fs::exists(path)) {
        std::ifstream file(path);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }
    }

    std::cerr << "Failed to open file: " << path << std::endl;
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

int main() {
    srand(time(0));
    crow::SimpleApp app;

    // Initialize Authentication
    std::unique_ptr<Authentication> auth;
    try {
        auth = std::make_unique<Authentication>();
        std::cout << "Authentication system initialized successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize authentication system: " << e.what() << std::endl;
        return 1;
    }

    // Handle OPTIONS requests for CORS
    CROW_ROUTE(app, "/<path>").methods("OPTIONS"_method)([](const crow::request& req, std::string) {
        auto res = crow::response(204);
        add_cors_headers(res, req);
        return res;
    });

    // --------- ROOT ROUTE ----------
    CROW_ROUTE(app, "/")([]() {
        std::string content = readFile("assets/index.html");
        if (content.empty()) {
            return crow::response(404, "Frontend not found");
        }
        auto res = crow::response(content);
        res.set_header("Content-Type", "text/html");
        return res;
    });

    // --------- SIGNUP ----------
    CROW_ROUTE(app, "/signup").methods("POST"_method)([&auth](const crow::request& req) {
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
            std::cout << "Signup successful for user: " << username << std::endl;
            return makeJsonResponse(req, 200, "Signup successful");

        } catch (const std::exception& e) {
            std::cerr << "Signup error: " << e.what() << std::endl;
            return makeJsonResponse(req, 400, e.what(), true);
        }
    });

    // --------- LOGIN ----------
    CROW_ROUTE(app, "/login").methods("POST"_method)([&auth](const crow::request& req) {
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
                return makeJsonResponse(req, 401, "Invalid username or password", true);
            }

            crow::json::wvalue result;
            result["token"] = token;
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

    std::cout << "\nServer running at http://localhost:18080" << std::endl;
    std::cout << "Database file: ../database/users.csv" << std::endl;
    std::cout << "Frontend file: ../assets/index.html" << std::endl;
    
    // Configure server to listen on all interfaces
    app.bindaddr("0.0.0.0")
       .port(18080)
       .multithreaded()
       .run();
    
    return 0;
}
