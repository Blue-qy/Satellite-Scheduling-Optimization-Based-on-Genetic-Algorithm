// csv_parser.h - 正确的方式
#pragma once
#include <vector>
#include <string>

class CSVParser {
public:
    static std::vector<std::vector<std::string>> parseCSV(const std::string& filename, char delimiter = ',');
    static std::vector<std::string> parseCSVLine(const std::string& line, char delimiter = ',');
};