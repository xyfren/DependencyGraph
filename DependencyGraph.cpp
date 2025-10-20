#include <iostream>
#include <tinyxml.h>
#include "ConfigLoader.h"

using namespace std;

int main()
{
    setlocale(0, "");

    ConfigLoader loader;

    string configFile = "config.xml";
   /* if (argc > 1) {
        configFile = argv[1];
    }*/

    if (loader.load(configFile)) {
        // Конфигурация успешно загружена
        // Можно использовать геттеры для работы с параметрами
        return 0;
    }
    else {
        return 1;
    }
}
