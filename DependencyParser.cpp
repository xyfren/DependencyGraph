#include "DependencyParser.h"

vector<string> DependencyParser::parse(string url) {
	vector<string> dependencyVector;
    try {
        dependencyVector = parsePackageMeta(url);
    }
    catch (exception& ex) {
        cerr << ex.what() << endl;
    }
    return dependencyVector;
}

size_t DependencyParser::writeCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
	size_t totalSize = size * nmemb;
	data->append((char*)contents, totalSize);
	return totalSize;
}

string DependencyParser::getJsonStr(string url) {
    CURL* curl;
    CURLcode res;
    string response;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0"); // Добавляем User-Agent

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            throw CurlParsingError("Ошибка Curl: " + string(curl_easy_strerror(res)));
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return response;
}

vector<string>  DependencyParser::parsePackageMeta(string packageUrl) {
    try {
        string packageMeta = getJsonStr(packageUrl);
        json j = json::parse(packageMeta);
        //cout << "JSON успешно раозбран" << endl;

        vector<string> dependencies;

        // Парсим зависимости из структуры crates.io API
        if (j.contains("crate") && j["crate"].contains("newest_version")) {
            string newest_version = j["crate"]["newest_version"];
            
            string depJsonStr = getJsonStr(packageUrl + "/" + newest_version + "/dependencies");
            json depJson = json::parse(depJsonStr);
            if (depJson.contains("dependencies")) {
                json deps = depJson["dependencies"];
                for (auto& d : deps) {
                    dependencies.push_back(string(d["crate_id"]));
                }

            }
        }
        else {
            throw JsonParsingError("JSON ошибка разбора: пакет не найден");
        }

        return dependencies;
    }
    catch (const json::parse_error& e) {
        throw JsonParsingError("JSON ошибка разбора: " + string(e.what()));
    }
}