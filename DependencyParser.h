#pragma once
#include <iostream>
#include <vector>
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

	static size_t writeCallback(void* contents, size_t size, size_t nmemb, string* data);
	string getJsonStr(string url);
    vector<string> parsePackageMeta(string url);

	public:
		vector<string> parse(string url);
};

