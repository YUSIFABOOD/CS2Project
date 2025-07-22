#ifndef REVIEW_H
#define REVIEW_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

class Review {
private:
    int reviewId;
    string bookTitle;
    string reviewer;
    string content;
    time_t timestamp;

public:
    Review(int id, const string& title, const string& user, const string& text);

    int getReviewId() const;
    string getBookTitle() const;
    string getReviewer() const;
    string getContent() const;
    time_t getTimestamp() const;

    void setContent(const string& newContent);

    json toJson() const;
    static Review fromJson(const json& j);
};

class ReviewManager {
private:
    vector<Review> reviews;
      string filePath;

public:
    ReviewManager(const string& path);

    void addReview(const string& bookTitle, const string& user, const string& text);
    void saveReviews();
    void loadReviews();

    vector<Review> searchByBook(const string& bookTitle) const;
    const vector<Review>& getAllReviews() const;
};

#endif
