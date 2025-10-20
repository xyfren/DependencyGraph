#pragma once

#include <iostream>
#include <string>
#include <algorithm>
#include "tinyxml2.h"

using namespace std;

class ConfigLoader {
private:
    struct Config {
       string package_name;
       string repository_url;
       bool test_repository_mode;
       string output_filename;
       string package_filter;
    };

    Config config;

    // Преобразование строки в bool
    bool stringToBool(const string& value) {
       string lowerValue = value;
       transform(lowerValue.begin(), lowerValue.end(), lowerValue.begin(), ::tolower);

        return (lowerValue == "true" || lowerValue == "1" || lowerValue == "yes");
    }

    // Вывод конфигурации
    void printConfig() {
       cout << "=== Параметры конфигурации ===" <<endl;
       cout << "package_name: " << config.package_name <<endl;
       cout << "repository_url: " << config.repository_url <<endl;
       cout << "test_repository_mode: " << (config.test_repository_mode ? "true" : "false") <<endl;
       cout << "output_filename: " << config.output_filename <<endl;
       cout << "package_filter: " << (config.package_filter.empty() ? "none" : config.package_filter) <<endl;
       cout << "=================================" <<endl;
    }

public:
    // Основной метод загрузки конфигурации
    bool load(const string& filename = "config.xml") {
        tinyxml2::XMLDocument doc;

        // Загрузка XML файла
        if (doc.LoadFile(filename.c_str()) != tinyxml2::XML_SUCCESS) {
           cerr << "ОШИБКА: Не удалось загрузить файл: " << filename <<endl;
            return false;
        }

        // Получение корневого элемента
        tinyxml2::XMLElement* root = doc.RootElement();
        if (!root) {
            cerr << "ОШИБКА: Ошибка отстутсвует корневой элемент" <<endl;
            return false;
        }

        // Парсинг всех полей
        //bool hasErrors = false;

        // package_name
        tinyxml2::XMLElement* packageElem = root->FirstChildElement("package_name");
        if (packageElem && packageElem->GetText()) {
            config.package_name = packageElem->GetText();
        }
        else {
           cerr << "ОШИБКА: Неверное значение package_name" <<endl;
           return false;
        }

        // repository_url
        tinyxml2::XMLElement* repoElem = root->FirstChildElement("repository_url");
        if (repoElem && repoElem->GetText()) {
            config.repository_url = repoElem->GetText();
        }
        else {
           cerr << "ОШИБКА: Неверное значение repository_url" <<endl;
           return false;
        }

        // test_repository_mode
        tinyxml2::XMLElement* testElem = root->FirstChildElement("test_repository_mode");
        if (testElem && testElem->GetText()) {
            config.test_repository_mode = stringToBool(testElem->GetText());
        }
        else {
           cerr << "ОШИБКА: Неверное значение test_repository_mode" <<endl;
           return false;
        }

        // output_filename
        tinyxml2::XMLElement* outputElem = root->FirstChildElement("output_filename");
        if (outputElem && outputElem->GetText()) {
            config.output_filename = outputElem->GetText();
        }
        else {
           cerr << "ОШИБКА: Неверное значение output_filename" <<endl;
           return false;
        }

        // package_filter (опциональное поле)
        tinyxml2::XMLElement* filterElem = root->FirstChildElement("package_filter");
        if (filterElem && filterElem->GetText()) {
            config.package_filter = filterElem->GetText();
        }
        else {
            config.package_filter = ""; // пустая строка если нет фильтра
        }

        // Вывод параметров (требование этапа 1)
        printConfig();
        cout << "Конфигурация успешно загружена!" <<endl;
        return true;
    }

    // Геттеры для доступа к конфигурации
   string getPackageName() const { return config.package_name; }
   string getRepositoryUrl() const { return config.repository_url; }
   bool getTestRepositoryMode() const { return config.test_repository_mode; }
   string getOutputFilename() const { return config.output_filename; }
   string getPackageFilter() const { return config.package_filter; }
};