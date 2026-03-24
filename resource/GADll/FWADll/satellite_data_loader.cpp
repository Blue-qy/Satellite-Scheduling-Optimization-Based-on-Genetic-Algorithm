// satellite_data_loader.cpp - 修复版本
#include "pch.h"
#include "satellite_data_loader.h"
#include <iostream>
#include <regex>
#include <stdexcept>
#include "csv_parser.h"
#include <filesystem>

SatelliteDataLoader::SatelliteDataLoader(const AlgorithmParams& params)
    : params_(params) {
    window_.resize(params_.num_satellites_, std::vector<int>(params_.total_minutes_, -1));
    idx_.resize(params_.num_satellites_);
}

int SatelliteDataLoader::parseTimeToMinutes(const std::string& timeStr) {
    auto timePoint = safeParseTime(timeStr);
    return timePoint.toMinutesSincestartAM(params_.starthour, params_.startminute, params_.startsecond);
}

SatelliteDataLoader::TimePoint SatelliteDataLoader::safeParseTime(const std::string& timeStr) {
    if (timeStr.empty() || timeStr == "NaN" || timeStr == "nan") {
        return TimePoint(-1, -1, -1);
    }

    try {
        std::vector<std::string> parts;
        std::stringstream ss(timeStr);
        std::string part;

        while (std::getline(ss, part, ',')) {
            parts.push_back(part);
        }

        std::string timePart = parts.size() > 1 ? parts[1] : parts[0];

        std::string cleanTimeStr = timePart;
        size_t dotPos = timePart.find('.');
        if (dotPos != std::string::npos) {
            cleanTimeStr = timePart.substr(0, dotPos);
        }

        std::regex pattern(R"((\d{1,2}):(\d{2}):(\d{2}))");
        std::smatch matches;

        if (std::regex_match(cleanTimeStr, matches, pattern)) {
            int hour = std::stoi(matches[1]);
            int minute = std::stoi(matches[2]);
            int second = std::stoi(matches[3]);

            return TimePoint(hour, minute, second);
        }
        else {
            std::cerr << "无法解析时间格式: " << cleanTimeStr << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "时间解析错误: " << timeStr << " - " << e.what() << std::endl;
    }

    return TimePoint(-1, -1, -1);
}

bool SatelliteDataLoader::loadDataFromExcel(const std::string& directoryPath) {
    std::cout << "开始加载卫星数据..." << std::endl;

    for (int i = 1; i <= params_.num_satellites_; ++i) {
        std::string file;
        if (!directoryPath.empty()) {
            std::filesystem::path dir(directoryPath);
            std::filesystem::path filename("satellite_" + std::to_string(i) + ".csv");
            std::filesystem::path fullPath = dir / filename;
            file = fullPath.string();
        }
        else {
            file = "satellite_" + std::to_string(i) + ".csv";
        }

        try {
            auto csv_data = CSVParser::parseCSV(file);

            size_t start_row = 0;
            if (!csv_data.empty() && csv_data[0][0] == "Date") {
                start_row = 1; // 跳过标题行
            }

            std::vector<double> coverage_values;
            std::vector<int> time_indices;
            int satellite_index = i - 1;

            for (size_t row = start_row; row < csv_data.size(); ++row) {
                const auto& fields = csv_data[row];

                if (fields.size() >= 3) {
                    std::string time_str = fields[1];

                    int minutes = parseTimeToMinutes(time_str);
                    if (minutes >= 0) {
                        time_indices.push_back(minutes);
                    }

                    std::string coverage_str = fields[2];
                    try {
                        double coverage = std::stod(coverage_str);
                        coverage_values.push_back(coverage);
                    }
                    catch (const std::exception& e) {
                        std::cerr << "覆盖率解析错误: " << coverage_str << " - " << e.what() << std::endl;
                    }
                }
            }

            idx_[satellite_index] = time_indices;
            coverage_data_[satellite_index] = coverage_values;

            for (int time_index : time_indices) {
                if (time_index >= 0 && time_index < params_.total_minutes_) {
                    window_[satellite_index][time_index] = 1;
                }
            }

            std::cout << "卫星 " << i << " 数据加载完成，时间点数量: " << time_indices.size()
                << ", 覆盖率数据数量: " << coverage_values.size() << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "加载文件 " << file << " 错误: " << e.what() << std::endl;
            continue;
        }
    }

    compressTimeWindows();

    std::cout << "所有卫星数据加载完成" << std::endl;
    return true;
}

void SatelliteDataLoader::compressTimeWindows() {
    std::cout << "开始压缩时间窗口..." << std::endl;

    std::vector<bool> all_minus_one_columns(params_.total_minutes_, true);

    for (int col = 0; col < params_.total_minutes_; ++col) {
        for (int row = 0; row < params_.num_satellites_; ++row) {
            if (window_[row][col] != -1) {
                all_minus_one_columns[col] = false;
                break;
            }
        }
    }

    for (int col = 0; col < params_.total_minutes_; ++col) {
        if (!all_minus_one_columns[col]) {
            bounds_.push_back(col);
        }
    }

    compressed_.resize(params_.num_satellites_);
    for (int row = 0; row < params_.num_satellites_; ++row) {
        compressed_[row].reserve(bounds_.size());
        for (int bound : bounds_) {
            compressed_[row].push_back(window_[row][bound]);
        }
    }

    std::cout << "时间窗口压缩完成，原始列数: " << params_.total_minutes_
        << ", 压缩后列数: " << bounds_.size() << std::endl;
}

void SatelliteDataLoader::saveCompressedData(const std::string& filename) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "无法创建文件: " << filename << std::endl;
        return;
    }

    file << "# bounds\n";
    for (size_t i = 0; i < bounds_.size(); ++i) {
        file << bounds_[i];
        if (i < bounds_.size() - 1) file << ",";
    }
    file << "\n";

    file << "# compressed\n";
    for (const auto& sat_schedule : compressed_) {
        for (size_t i = 0; i < sat_schedule.size(); ++i) {
            file << sat_schedule[i];
            if (i < sat_schedule.size() - 1) file << ",";
        }
        file << "\n";
    }

    file.close();
    std::cout << "压缩数据已保存到: " << filename << std::endl;
}