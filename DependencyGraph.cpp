#include <iostream>
#include <tinyxml2.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "ConfigLoader.h"
#include "DependencyParser.h"

using namespace std;

int main()
{
    setlocale(0, "");

    ConfigLoader loader;

    string configFile = "config.xml";

    if (!loader.load(configFile)) {
        return 1;
    }

    string packageName = loader.getPackageName();
    string repositoryUrl = loader.getRepositoryUrl();
    bool testRepositoryMode = loader.getTestRepositoryMode();
    string packageFilter = loader.getPackageFilter();

    DependencyParser dp(repositoryUrl, testRepositoryMode);

    vector<string> filters;
    if (!packageFilter.empty()) {
        filters.push_back(packageFilter);
    }

    vector<string> deps = dp.getPackageDependenciesTransitive(packageName, filters);

    if (deps.empty()) {
        cout << "Зависимостей не найдено:(" << endl;
        return 0;
    }

    cout << "Транзитивные зависимости пакета " << packageName << ":" << endl;
    for (string& dep : deps) {
        cout << dep << endl;
    }

    return 0;
}