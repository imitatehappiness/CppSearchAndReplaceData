#include <iostream>
#include <chrono>
#include <memory>

#include "searchreplacedata.h"

/*!
\brief  Программа производит поиск и замену данных в шаблонах в произвольно вложенном каталоге файлов и папок. 
        Для этого она парсит каталог, затем в каждом файле заменяет все вхождения заданных шаблонов на соответствующий текст замены. 
        Программа работает в несколько потоков и сообщает пользователю о выполненной работе. 
        Конфигурационный файл содержит информацию о количестве потоков, адресе корневой директории и перечислении шаблонов и их замен.
*/
int main() {
    std::string configFilePath = "config.txt";

    auto startTime = std::chrono::high_resolution_clock::now();

    std::unique_ptr<SearchReplaceData> ptr(new SearchReplaceData(configFilePath));

    if (!ptr->parseConfig()) {
        std::cerr << "Unable to open config file: " << configFilePath << std::endl;
        return -1;
    }

    ptr->parseDirectory();
    std::cout << "Job is done." << std::endl;

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    std::cout << "time: " << duration.count() << " mc" << std::endl;

    return 0;
}
