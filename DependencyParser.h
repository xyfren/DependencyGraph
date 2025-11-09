#pragma once
#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

class CurlParsingError : public exception {
private:
    string message;

public:
    CurlParsingError(const string& msg) : message(msg) {}

    const char* what() const noexcept override {
        return message.c_str();
    }
};

class JsonParsingError : public exception {
private:
    string message;

public:
    JsonParsingError(const string& msg) : message(msg) {}

    const char* what() const noexcept override {
        return message.c_str();
    }
};

class DependencyParser
{
    string m_repositoryUrl;
    bool m_testRepositoryMode;
    unordered_map<string, vector<string>> m_dependencyCache;

    static size_t writeCallback(void* contents, size_t size, size_t nmemb, string* data);
    string getMeta(string packageUrl);
    vector<string> parsePackageMeta(string packageUrl);
    vector<string> parse(string packageUrl);

    // Рекурсивный DFS для транзитивных зависимостей
    void getTransitiveDependenciesDFS(const string& packageName,
        vector<string>& result,
        unordered_set<string>& visited,
        string currentPath,
        const vector<string>& filters,
        int depth);

public:
    DependencyParser(string repositoryUrl, bool testRepositoryMode);
    vector<string> getPackageDependencies(string packageName);
    vector<string> getPackageDependenciesTransitive(string packageName, vector<string> filters = {});
};