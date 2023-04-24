#pragma once

#include <string>
#include <vector>

/*!
 \brief Структура configFileData содержит данные, прочитанные из конфигурационного файла:
 количество потоков numThreads, корневой каталог rootDir и список замен replacements,
 который представляет собой вектор пар строк (старое значение и новое значение) для замены в файлах.
*/
struct ConfigFileData {
    int numThreads;
    std::string rootDir;
    std::vector<std::pair<std::string, std::string>> replacements;
};


/*!
\class SearchReplaceData
\brief Класс SearchReplaceData осуществляет поиск и замену данных в шаблонах.
*/
class SearchReplaceData {
public:
    // Конструктор с параметрами
    SearchReplaceData(const std::string& configFilePath);
    // Деструктор
    ~SearchReplaceData();
    // Парсинг файла 
    void parseFile(const std::string& path);
    // Парсинг директории
    void parseDirectory();
    // Парсинг конфиг файла
    bool parseConfig();
private:
    // Объект структуры, хранящий данные из конфиг файла
    ConfigFileData* mConfigFileData;
    // Путь до конфиг файла
    std::string mConfigFilePath;
};



