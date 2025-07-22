#include "review.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

Review::Review(int id, const string& title, const string& user, const string& text)
    : reviewId(id), bookTitle(title), reviewer(user), content(text), timestamp(time(nullptr)) {}

int Review::getReviewId() const { return reviewId; }
string Review::getBookTitle() const { return bookTitle; }
string Review::getReviewer() const { return reviewer; }
string Review::getContent() const { return content; }
time_t Review::getTimestamp() const { return timestamp; }

void Review::setContent(const string& newContent) {
    content = newContent;
    timestamp = time(nullptr);
}

json Review::toJson() const {
    return {
        {"id", reviewId},
        {"bookTitle", bookTitle},
        {"reviewer", reviewer},
        {"content", content},
        {"timestamp", timestamp}
    };
}

Review Review::fromJson(const json& j) {
    Review r(j["id"], j["bookTitle"], j["reviewer"], j["content"]);
    r.timestamp = j["timestamp"];
    return r;
}


ReviewManager::ReviewManager(const string& path) : nextReviewId(1), filePath(path) {
    loadReviews();
}

void ReviewManager::addReview(const string& bookTitle, const string& user, const string& text) {
    Review review(nextReviewId++, bookTitle, user, text);
    reviews.push_back(review);
    saveReviews();
}

void ReviewManager::saveReviews() {
    json j = json::array();
    for (const auto& review : reviews) {
        j.push_back(review.toJson());
    }

    ofstream file(filePath);
    if (file.is_open()) {
        file << j.dump(4);
    }
}

void ReviewManager::loadReviews() {
    ifstream file(filePath);
    if (!file.is_open() || fs::is_empty(filePath)) return;

    json j;
    file >> j;

    for (const auto& item : j) {
        reviews.push_back(Review::fromJson(item));
    }
}

vector<Review> ReviewManager::searchByBook(const string& bookTitle) const {
    vector<Review> result;
    for (const auto& r : reviews) {
        if (r.getBookTitle() == bookTitle) {
            result.push_back(r);
        }
    }
    return result;
}

const vector<Review>& ReviewManager::getAllReviews() const {
    return reviews;
}
