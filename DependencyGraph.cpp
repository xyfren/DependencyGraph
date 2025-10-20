#include <iostream>
#include <tinyxml.h>
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

    DependencyParser dp;

    vector<string> deps = dp.parse(loader.getRepositoryUrl());

    if (deps.empty()) {
        cout << "Зависимостей не найдено:(" << endl;
        return 0;
    }
    cout << "Зависимости пакета " << loader.getPackageName() << ":" << endl;
    for (string& dep : deps) {
        cout << dep << endl;
    }
    return 0;
}
