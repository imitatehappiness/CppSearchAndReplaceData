#pragma once

#include <string>
#include <vector>

/*!
 \brief ��������� configFileData �������� ������, ����������� �� ����������������� �����:
 ���������� ������� numThreads, �������� ������� rootDir � ������ ����� replacements,
 ������� ������������ ����� ������ ��� ����� (������ �������� � ����� ��������) ��� ������ � ������.
*/
struct ConfigFileData {
    int numThreads;
    std::string rootDir;
    std::vector<std::pair<std::string, std::string>> replacements;
};


/*!
\class SearchReplaceData
\brief ����� SearchReplaceData ������������ ����� � ������ ������ � ��������.
*/
class SearchReplaceData {
public:
    // ����������� � �����������
    SearchReplaceData(const std::string& configFilePath);
    // ����������
    ~SearchReplaceData();
    // ������� ����� 
    void parseFile(const std::string& path);
    // ������� ����������
    void parseDirectory();
    // ������� ������ �����
    bool parseConfig();
private:
    // ������ ���������, �������� ������ �� ������ �����
    ConfigFileData* mConfigFileData;
    // ���� �� ������ �����
    std::string mConfigFilePath;
};



