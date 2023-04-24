#include "pch.h"

#include <iostream>
#include <fstream>
#include <sstream>

/*!
\brief Функция считывает содержимое двух файлов в строковые переменные и возвращает их в виде пары значений типа std::string.  
\param path1 путь до измененного алгоритмом файла
\param path2 путь до файла для проверки
\return Объект std::pair<std::string, std::string>, 
       содержащий две строки: text1 и text2, которые содержат содержимое файлов, соответствующих путям path1 и path2.
\details Данная функция getText принимает два аргумента типа const std::string&: path1 и path2, которые представляют пути к двум файлам. 
         Функция открывает каждый файл с помощью std::ifstream и проверяет, успешно ли произошло открытие. 
         Если открытие не удалось, функция выводит сообщение об ошибке в стандартный поток ошибок std::cerr.

         Далее, функция создает два объекта типа std::stringstream (ss1 и ss2) и записывает содержимое каждого файла 
         в соответствующий объект std::stringstream. Затем, содержимое каждого файла из std::stringstream присваивается соответствующей 
         строковой переменной (text1 и text2). Функция возвращает объект std::pair<std::string, std::string>, 
         содержащий две строки: text1 и text2, которые содержат содержимое файлов, соответствующих путям path1 и path2.
*/
std::pair<std::string, std::string> getText(const std::string& path1, const std::string& path2){

    std::ifstream file1(path1);
    if (!file1.is_open()) {
        std::cerr << "Failed to open file: " << path1 << std::endl;
    }

    std::ifstream file2(path2);
    if (!file2.is_open()) {
        std::cerr << "Failed to open file: " << path2 << std::endl;
    }

    std::string text1;
    std::string text2;

    std::stringstream ss1;
    ss1 << file1.rdbuf();
    text1 = ss1.str();

    std::stringstream ss2;
    ss2 << file2.rdbuf();
    text2 = ss2.str();

    return std::pair<std::string, std::string>(text1, text2);
}

TEST(TextComparisonTest, TextAreEqual1OnlyTemplates) {
    std::string path1 = "C:/Users/imitatehappiness/Desktop/CppSearchAndReplaceData/TestData/data/1/1.1/test.txt";
    std::string path2 = "C:/Users/imitatehappiness/Desktop/CppSearchAndReplaceData/TestData/dataforcomporation/1/1.1/test.txt";

    std::pair<std::string, std::string> texts = getText(path1, path2);
    ASSERT_STREQ(texts.first.c_str(), texts.second.c_str());
}

TEST(TextComparisonTest, TextAreEqual2OnlyTemplates) {
    std::string path1 = "C:/Users/imitatehappiness/Desktop/CppSearchAndReplaceData/TestData/data/1/1.1/test1.txt";
    std::string path2 = "C:/Users/imitatehappiness/Desktop/CppSearchAndReplaceData/TestData/dataforcomporation/1/1.1/test1.txt";

    std::pair<std::string, std::string> texts = getText(path1, path2);
    ASSERT_STREQ(texts.first.c_str(), texts.second.c_str());
}

TEST(TextComparisonTest, TextAreEqual3LargeText) {
    std::string path1 = "C:/Users/imitatehappiness/Desktop/CppSearchAndReplaceData/TestData/data/1/test.txt";
    std::string path2 = "C:/Users/imitatehappiness/Desktop/CppSearchAndReplaceData/TestData/dataforcomporation/1/test.txt";

    std::pair<std::string, std::string> texts = getText(path1, path2);
    ASSERT_STREQ(texts.first.c_str(), texts.second.c_str());
}

TEST(TextComparisonTest, TextAreEqual4Empty) {
    std::string path1 = "C:/Users/imitatehappiness/Desktop/CppSearchAndReplaceData/TestData/data/1/test1.txt";
    std::string path2 = "C:/Users/imitatehappiness/Desktop/CppSearchAndReplaceData/TestData/dataforcomporation/1/test1.txt";

    std::pair<std::string, std::string> texts = getText(path1, path2);
    ASSERT_STREQ(texts.first.c_str(), texts.second.c_str());
}

TEST(TextComparisonTest, TextAreEqual5WithoutTemplates) {
    std::string path1 = "C:/Users/imitatehappiness/Desktop/CppSearchAndReplaceData/TestData/data/1/test2.txt";
    std::string path2 = "C:/Users/imitatehappiness/Desktop/CppSearchAndReplaceData/TestData/dataforcomporation/1/test2.txt";

    std::pair<std::string, std::string> texts = getText(path1, path2);
    ASSERT_STREQ(texts.first.c_str(), texts.second.c_str());
}

TEST(TextComparisonTest, TextAreEqual6WithError) {
    std::string path1 = "C:/Users/imitatehappiness/Desktop/CppSearchAndReplaceData/TestData/data/1/test2.txt";
    std::string path2 = "C:/Users/imitatehappiness/Desktop/CppSearchAndReplaceData/TestData/dataforcomporation/1/test2.txt";

    std::pair<std::string, std::string> texts = getText(path1, path2);
    ASSERT_STREQ(texts.first.c_str(), "error");
}