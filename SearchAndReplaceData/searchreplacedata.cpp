#include "searchreplacedata.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <filesystem> // В стандарте C++17 эта библиотека включена в стандартную библиотеку языка.
#include <future>
#include <sstream>

namespace fs = std::filesystem;

/*!
 \brief Конструктор.
 \param configFilePath путь до конфиг файла.
 \details инициализация mConfigFilePath(путь до конфиг файла) и mConfigFileData(Объект структуры, хранящий данные из конфиг файла).
 */
SearchReplaceData::SearchReplaceData(const std::string& configFilePath)
    : mConfigFilePath(configFilePath) {
    mConfigFileData = new ConfigFileData();
}

/*!
 \brief Деструктор.
 \details Очистка mConfigFileData.
 */
SearchReplaceData::~SearchReplaceData() {
    delete mConfigFileData;
}

/*!
 \brief parseFile функция открывает файл, заменяет все значения в списке замен replacements
        и записывает изменения в тот же файл.
 \param path путь до файла
 \details 1. Чтение файлов: Сначала создается объект ifstream для открытия файла (путь к которому передается в параметре функции).
             Если файл не удалось открыть, выводится сообщение об ошибке и функция завершается.
             Затем создаются необходимые переменные: мьютекс (mutex), который будет использоваться для блокировки доступа к файлу из нескольких потоков,
             количество потоков, которые будут использоваться для чтения файла, и вектор futures для хранения результатов асинхронных операций чтения файла в строку.
             Далее в цикле создаются задачи на чтение файла в строку в несколько потоков с помощью функции std::async и добавляются в вектор futures.
             Каждая задача выполняется в отдельном потоке, блокируя мьютекс для доступа к файлу.
             После завершения выполнения всех задач из вектора futures полученные строки конкатенируются в одну строку.
             После чтения файла файл закрывается.
          2. Замена шаблонов: Сначала из конфигурационного файла получаются все необходимые пары значений для замены,
             которые сохраняются в векторе replacements.
             Затем, для каждой пары значений из вектора производится замена всех вхождений шаблона r.first в строке contents на соответствующее значение r.second.
             Для этого используется цикл while, который проходит по всем вхождениям шаблона в строке.
             Внутри цикла находится вызов метода replace(), который заменяет вхождение шаблона в строке на соответствующее значение и обновляет позицию,
             на которой нужно начать следующий поиск, увеличивая ее на длину замененного текста.
             После завершения замен всех шаблонов, строка contents содержит измененный текст.
*/
void SearchReplaceData::parseFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()){
        std::cout << "Failed to open file: " << path << std::endl;
        return;
    }
    std::mutex m;
    int numTreads = mConfigFileData->numThreads;
    std::vector<std::future<std::string>> futures;

    for (int i = 0; i < numTreads; i++) {
        futures.push_back(std::async(std::launch::async, [&file, &m]() {
            std::lock_guard<std::mutex> lock(m);
            std::stringstream ss;
            ss << file.rdbuf();
            return ss.str();
            }));
    }

    std::string contents = "";
    for (auto& future : futures) {
        contents += future.get();
    }

    file.close();

    std::vector<std::pair<std::string, std::string>> replacements = mConfigFileData->replacements;

    for (auto r : replacements) {
        size_t pos = 0;
        while ((pos = contents.find(r.first, pos)) != std::string::npos) {
            contents.replace(pos, r.first.length(), r.second);
            pos += r.second.length();
        }
    }

    std::ofstream outfile(path);
    if (!outfile.is_open()) {
        std::cout << "Failed to open file: " << path << std::endl;
        return;
    }
    outfile << contents;
    outfile.close();
}

/*!
 \brief parseDirectory функция проходит рекурсивно по всем файлам в указанном каталоге
        и запускает parseFile для каждого обнаруженного файла.
 \details Данная функция отвечает за обход и парсинг файлов в указанной директории и её поддиректориях.
          Сначала из конфигурационного файла получается количество потоков numThreads и адрес корневой директории path.
          Затем создается вектор потоков threads, в который будут добавляться потоки для парсинга каждого файла.

          Далее начинается рекурсивный обход всех файлов в директории и ее поддиректориях.
          Если файл не является обычным файлом, то он пропускается.
          Для каждого обычного файла создается поток, который запускает функцию parseFile для его парсинга.
          При достижении максимального количества потоков (numThreads) все запущенные потоки завершаются и освобождаются,
          а новые потоки для парсинга файлов начинаются с новой порции файлов.

          После того, как были запущены потоки для всех файлов, которые необходимо обработать,
          оставшиеся потоки завершают свою работу и освобождаются.
*/
void SearchReplaceData::parseDirectory() {
    int numThreads = mConfigFileData->numThreads;
    std::string path = mConfigFileData->rootDir;
    std::vector< std::thread> threads;
    if (!std::filesystem::exists(path)){
        std::cerr << "Error opening the directory: " << path << std::endl;
        return;
    }
    for (auto& entry : fs::recursive_directory_iterator(path)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        std::string entryPath = entry.path().string();
        threads.push_back(std::thread(&SearchReplaceData::parseFile, this, entryPath));
        if (static_cast<int>(threads.size()) >= numThreads) {
            for (auto& t : threads) {
                t.join();
            }
            threads.clear();
        }
    }
    for (auto& t : threads) {
        t.join();
    }
}

/*!
 \brief parseConfig функция читает конфигурационный файл
        и заполняет структуру configFileData данными из конфигурационного файла.
 \return true, если конфигурационный файл был успешно прочитан, и false в противном случае.
*/
bool SearchReplaceData::parseConfig() {
    std::ifstream config(mConfigFilePath);
    if (!std::filesystem::exists(mConfigFilePath)) {
        std::cerr << "Error opening the directory: " << mConfigFilePath << std::endl;
        return false;
    }
    if (config.is_open()) {
        std::string line;
        while ( std::getline(config, line)) {
            if (line.empty()) {
                continue;
            }
            size_t delim = line.find('=');
            if (delim == std::string::npos) {
                std::cerr << "Invalid line in config file: " << line << std::endl;
                continue;
            }
            std::string key = line.substr(0, delim);
            std::string value = line.substr(delim + 1);
            if (key == "numThreads") {
                mConfigFileData->numThreads = stoi(value);
            }
            else if (key == "rootDir") {
                mConfigFileData->rootDir = value;
            }
            else {
                mConfigFileData->replacements.push_back({ key, value });
            }
        }
        config.close();
    }
    else {
        return false;
    }

    return true;
}


