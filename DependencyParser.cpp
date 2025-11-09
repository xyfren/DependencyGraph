#include "DependencyParser.h"

DependencyParser::DependencyParser(string repositoryUrl, bool testRepositoryMode)
    : m_repositoryUrl(repositoryUrl), m_testRepositoryMode(testRepositoryMode) {}

size_t DependencyParser::writeCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    size_t totalSize = size * nmemb;
    data->append((char*)contents, totalSize);
    return totalSize;
}

string DependencyParser::getMeta(string packageUrl) {
    // Режим тестового репозитория - заглушка
    if (m_testRepositoryMode) {
        // В тестовом режиме эмулируем простой граф зависимостей
        if (packageUrl.find("A") != string::npos) return R"({"dependencies": ["B", "C"]})";
        if (packageUrl.find("B") != string::npos) return R"({"dependencies": ["C", "D"]})";
        if (packageUrl.find("C") != string::npos) return R"({"dependencies": ["E"]})";
        if (packageUrl.find("D") != string::npos) return R"({"dependencies": []})";
        if (packageUrl.find("E") != string::npos) return R"({"dependencies": []})";
        return R"({"dependencies": []})";
    }

    CURL* curl;
    CURLcode res;
    string response;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, packageUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            throw CurlParsingError("Ошибка Curl: " + string(curl_easy_strerror(res)));
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return response;
}

vector<string> DependencyParser::parsePackageMeta(string packageUrl) {
    try {
        // Режим тестового репозитория
        if (m_testRepositoryMode) {
            string packageMeta = getMeta(packageUrl);
            json j = json::parse(packageMeta);

            vector<string> dependencies;
            if (j.contains("dependencies")) {
                for (const auto& dep : j["dependencies"]) {
                    dependencies.push_back(dep);
                }
            }
            return dependencies;
        }

        // Реальный репозиторий (оригинальная логика)
        string packageMeta = getMeta(packageUrl);
        json j = json::parse(packageMeta);

        vector<string> dependencies;

        if (j.contains("crate") && j["crate"].contains("newest_version")) {
            string newest_version = j["crate"]["newest_version"];

            string depJsonStr = getMeta(packageUrl + "/" + newest_version + "/dependencies");
            json depJson = json::parse(depJsonStr);

            if (depJson.contains("dependencies")) {
                json deps = depJson["dependencies"];
                for (auto& d : deps) {
                    bool isOptional = d.contains("optional") && d["optional"] == true;
                    bool isDev = d.contains("kind") && string(d["kind"]) == "dev";

                    // НИКОГДА не включаем optional зависимости
                    if (isOptional) {
                        continue;
                    }

                    // НИКОГДА не включаем dev-зависимости
                    if (isDev) {
                        continue;
                    }

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

vector<string> DependencyParser::parse(string packageUrl) {
    vector<string> dependencyVector;
    try {
        dependencyVector = parsePackageMeta(packageUrl);
    }
    catch (exception& ex) {
        cerr << ex.what() << endl;
    }
    return dependencyVector;
}

vector<string> DependencyParser::getPackageDependencies(string packageName) {
    if (m_dependencyCache.find(packageName) != m_dependencyCache.end()) {
        return m_dependencyCache[packageName];
    }

    string packageUrl;
    if (m_testRepositoryMode) {
        packageUrl = packageName; // В тестовом режиме используем имя пакета как URL
    }
    else {
        packageUrl = m_repositoryUrl + packageName;
    }

    vector<string> dependencies = parse(packageUrl);

    m_dependencyCache[packageName] = dependencies;
    return dependencies;
}

void DependencyParser::getTransitiveDependenciesDFS(const string& packageName,
    vector<string>& result,
    unordered_set<string>& visited,
    string currentPath,
    const vector<string>& filters,
    int depth) {
    const int MAX_DEPTH = 10; // Защита от бесконечной рекурсии

    if (depth > MAX_DEPTH) {
        cout << "WARNING: Превышена максимальная глубина рекурсии (" << MAX_DEPTH << ")" << endl;
        return;
    }

    // Проверка циклических зависимостей
    if (visited.find(packageName) != visited.end()) {
        cout << "Обнаружена циклическая зависимость: " << packageName << " в пути " << currentPath << endl;
        return;
    }

    visited.insert(packageName);

    vector<string> dependencies = getPackageDependencies(packageName);

    for (const string& dep : dependencies) {
        // Проверяем фильтры
        bool filtered = false;
        for (const string& filter : filters) {
            if (dep.find(filter) != string::npos) {
                filtered = true;
                break;
            }
        }
        if (filtered) continue;

        string newPath = currentPath + "/" + dep;
        result.push_back(newPath);

        // Рекурсивный вызов для транзитивных зависимостей
        getTransitiveDependenciesDFS(dep, result, visited, newPath, filters, depth + 1);
    }

    visited.erase(packageName);
}

vector<string> DependencyParser::getPackageDependenciesTransitive(string packageName, vector<string> filters) {
    vector<string> result;
    unordered_set<string> visited;

    cout << "Построение графа зависимостей для пакета: " << packageName << endl;

    // Начинаем DFS с корневого пакета
    getTransitiveDependenciesDFS(packageName, result, visited, packageName, filters, 0);

    return result;
}