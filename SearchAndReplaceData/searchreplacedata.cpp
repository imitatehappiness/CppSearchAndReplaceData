#include "searchreplacedata.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <filesystem> // � ��������� C++17 ��� ���������� �������� � ����������� ���������� �����.
#include <future>
#include <sstream>

using namespace std;
namespace fs = std::filesystem;

/*!
 \brief �����������.
 \param configFilePath ���� �� ������ �����.
 \details ������������� mConfigFilePath(���� �� ������ �����) � mConfigFileData(������ ���������, �������� ������ �� ������ �����).
 */
SearchReplaceData::SearchReplaceData(const string& configFilePath)
    : mConfigFilePath(configFilePath) {
    mConfigFileData = new ConfigFileData();
}

/*!
 \brief ����������.
 \details ������� mConfigFileData.
 */
SearchReplaceData::~SearchReplaceData() {
    delete mConfigFileData;
}

/*!
 \brief parseFile ������� ��������� ����, �������� ��� �������� � ������ ����� replacements
        � ���������� ��������� � ��� �� ����.
 \param path ���� �� �����
 \details 1. ������ ������: ������� ��������� ������ ifstream ��� �������� ����� (���� � �������� ���������� � ��������� �������).
             ���� ���� �� ������� �������, ��������� ��������� �� ������ � ������� �����������.
             ����� ��������� ����������� ����������: ������� (mutex), ������� ����� �������������� ��� ���������� ������� � ����� �� ���������� �������,
             ���������� �������, ������� ����� �������������� ��� ������ �����, � ������ futures ��� �������� ����������� ����������� �������� ������ ����� � ������.
             ����� � ����� ��������� ������ �� ������ ����� � ������ � ��������� ������� � ������� ������� std::async � ����������� � ������ futures.
             ������ ������ ����������� � ��������� ������, �������� ������� ��� ������� � �����.
             ����� ���������� ���������� ���� ����� �� ������� futures ���������� ������ ��������������� � ���� ������.
             ����� ������ ����� ���� �����������.
          2. ������ ��������: ������� �� ����������������� ����� ���������� ��� ����������� ���� �������� ��� ������,
             ������� ����������� � ������� replacements.
             �����, ��� ������ ���� �������� �� ������� ������������ ������ ���� ��������� ������� r.first � ������ contents �� ��������������� �������� r.second.
             ��� ����� ������������ ���� while, ������� �������� �� ���� ���������� ������� � ������.
             ������ ����� ��������� ����� ������ replace(), ������� �������� ��������� ������� � ������ �� ��������������� �������� � ��������� �������,
             �� ������� ����� ������ ��������� �����, ���������� �� �� ����� ����������� ������.
             ����� ���������� ����� ���� ��������, ������ contents �������� ���������� �����.
*/
void SearchReplaceData::parseFile(const string& path) {
    ifstream file(path);
    if (!file.is_open()){
        std::cout << "Failed to open file: " << path << std::endl;
        return;
    }
    mutex m;
    int numTreads = mConfigFileData->numThreads;
    std::vector<std::future<std::string>> futures;

    for (int i = 0; i < numTreads; i++) {
        futures.push_back(std::async(std::launch::async, [&file, &m]() {
            lock_guard<mutex> lock(m);
            std::stringstream ss;
            ss << file.rdbuf();
            return ss.str();
            }));
    }

    string contents = "";
    for (auto& future : futures) {
        contents += future.get();
    }

    file.close();

    std::vector<std::pair<std::string, std::string>> replacements = mConfigFileData->replacements;

    for (auto r : replacements) {
        size_t pos = 0;
        while ((pos = contents.find(r.first, pos)) != string::npos) {
            contents.replace(pos, r.first.length(), r.second);
            pos += r.second.length();
        }
    }

    ofstream outfile(path);
    if (!outfile.is_open()) {
        std::cout << "Failed to open file: " << path << std::endl;
        return;
    }
    outfile << contents;
    outfile.close();
}

/*!
 \brief parseDirectory ������� �������� ���������� �� ���� ������ � ��������� ��������
        � ��������� parseFile ��� ������� ������������� �����.
 \details ������ ������� �������� �� ����� � ������� ������ � ��������� ���������� � � ��������������.
          ������� �� ����������������� ����� ���������� ���������� ������� numThreads � ����� �������� ���������� path.
          ����� ��������� ������ ������� threads, � ������� ����� ����������� ������ ��� �������� ������� �����.

          ����� ���������� ����������� ����� ���� ������ � ���������� � �� ��������������.
          ���� ���� �� �������� ������� ������, �� �� ������������.
          ��� ������� �������� ����� ��������� �����, ������� ��������� ������� parseFile ��� ��� ��������.
          ��� ���������� ������������� ���������� ������� (numThreads) ��� ���������� ������ ����������� � �������������,
          � ����� ������ ��� �������� ������ ���������� � ����� ������ ������.

          ����� ����, ��� ���� �������� ������ ��� ���� ������, ������� ���������� ����������,
          ���������� ������ ��������� ���� ������ � �������������.
*/
void SearchReplaceData::parseDirectory() {
    int numThreads = mConfigFileData->numThreads;
    string path = mConfigFileData->rootDir;
    vector<thread> threads;
    if (!std::filesystem::exists(path)){
        cerr << "Error opening the directory: " << path << endl;
        return;
    }
    for (auto& entry : fs::recursive_directory_iterator(path)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        string entryPath = entry.path().string();
        threads.push_back(thread(&SearchReplaceData::parseFile, this, entryPath));
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
 \brief parseConfig ������� ������ ���������������� ����
        � ��������� ��������� configFileData ������� �� ����������������� �����.
 \return true, ���� ���������������� ���� ��� ������� ��������, � false � ��������� ������.
*/
bool SearchReplaceData::parseConfig() {
    ifstream config(mConfigFilePath);

    if (config.is_open()) {
        string line;
        while (getline(config, line)) {
            if (line.empty()) {
                continue;
            }
            size_t delim = line.find('=');
            if (delim == string::npos) {
                cerr << "Invalid line in config file: " << line << endl;
                continue;
            }
            string key = line.substr(0, delim);
            string value = line.substr(delim + 1);
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


