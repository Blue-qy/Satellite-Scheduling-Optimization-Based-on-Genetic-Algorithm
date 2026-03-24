// csv_reader_optimized.h - ÐÞ¸´ºó
#pragma once
#include <vector>
#include <string>

class OptimizedCSVReader {
public:
    static std::vector<bool> readSingleColumnCSV(const std::string& filepath, int expected_rows = -1);

private:
    static std::string trim(const std::string& str);
    static bool parseBoolValue(const std::string& str);
};
