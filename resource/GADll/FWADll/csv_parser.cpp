// csv_parser.cpp

/*
* csv_parser.h/cpp: 通用CSV解析器
解析CSV文件为字符串矩阵
处理分隔符和空行
*/

#include "pch.h"
#include "csv_parser.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

// 实现静态成员函数 - 注意：这里不需要写static关键字
std::vector<std::vector<std::string>> CSVParser::parseCSV(const std::string& filename, char delimiter) {
    std::vector<std::vector<std::string>> result;
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("无法打开CSV文件: " + filename);
    }

    std::string line;
    while (std::getline(file, line)) {
        // 跳过空行
        if (line.empty() || (line.find_first_not_of(" \t\r\n") == std::string::npos)) {
            continue;
        }

        auto row = parseCSVLine(line, delimiter);
        if (!row.empty()) {
            result.push_back(row);
        }
    }

    return result;
}

std::vector<std::string> CSVParser::parseCSVLine(const std::string& line, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(line);
    std::string cell;

    while (std::getline(ss, cell, delimiter)) {
        result.push_back(cell);
    }

    // 处理最后一个字段可能为空的情况
    if (!line.empty() && line.back() == delimiter) {
        result.push_back("");
    }

    return result;
}