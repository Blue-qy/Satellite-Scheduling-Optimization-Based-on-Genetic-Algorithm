// main.cpp - 卫星调度优化系统控制台应用程序入口

#include "pch.h"

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <filesystem>
#include "satellite_scheduler_api.h"
#include <random>          // 添加这行
#include <fstream>         // 添加这行

// 测试数据生成函数（用于调试）
void GenerateTestDataIfNeeded() {
    std::cout << "[调试] 检查测试数据..." << std::endl;

    // 检查是否已有卫星数据文件
    bool hasData = true;
    for (int i = 1; i <= 3; ++i) {
        std::string filename = "satellite_" + std::to_string(i) + ".csv";
        if (!std::filesystem::exists(filename)) {
            hasData = false;
            break;
        }
    }

    if (!hasData) {
        std::cout << "[调试] 未找到测试数据，生成简单测试数据..." << std::endl;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> time_dist(0, 720);
        std::uniform_real_distribution<> coverage_dist(0.1, 0.9);

        for (int sat_id = 1; sat_id <= 3; ++sat_id) {
            std::string filename = "satellite_" + std::to_string(sat_id) + ".csv";
            std::ofstream file(filename);

            if (file.is_open()) {
                // 写入标题行
                file << "Date,Time,Coverage,Other\n";

                // 生成50个随机数据点
                for (int i = 0; i < 50; ++i) {
                    int time_minutes = time_dist(gen);
                    int hour = 8 + time_minutes / 60;
                    int minute = time_minutes % 60;
                    double coverage = coverage_dist(gen);

                    file << "1 May 2025,";
                    file << std::setw(2) << std::setfill('0') << hour << ":";
                    file << std::setw(2) << std::setfill('0') << minute << ":00.000,";
                    file << std::fixed << std::setprecision(3) << coverage << ",";
                    file << coverage << "\n";
                }

                file.close();
                std::cout << "[调试] 生成测试文件: " << filename << std::endl;
            }
        }
    }
    else {
        std::cout << "[调试] 已找到测试数据文件" << std::endl;
    }
}

// 显示欢迎信息
void ShowWelcome() {
    std::cout << "\n";
    std::cout << "==================================================\n";
    std::cout << "   卫星调度优化系统 - 控制台测试版本\n";
    std::cout << "==================================================\n";
    std::cout << "\n";
}

// 显示结果
void DisplayResults(void* scheduler) {
    std::cout << "\n[结果分析]\n";
    std::cout << "==================================================\n";

    int solution_count = GetSolutionCount(scheduler);
    std::cout << "优化结果: 共找到 " << solution_count << " 个有效解决方案\n\n";

    if (solution_count == 0) {
        std::cout << "警告: 未找到有效解决方案！\n";
        std::cout << "错误信息: " << GetSchedulerLastError() << std::endl;
        return;
    }

    for (int i = 0; i < solution_count; ++i) {
        int satellite_count = 0;
        double coverage = 0.0;
        double variance = 0.0;

        if (GetSolutionInfo(scheduler, i, &satellite_count, &coverage, &variance)) {
            std::cout << "解决方案 #" << i + 1 << ":\n";
            std::cout << "  ├─ 使用的卫星数量: " << satellite_count << " 颗\n";
            std::cout << "  ├─ 覆盖率时间积: " << std::fixed << std::setprecision(4) << "\n";
            std::cout << "  └─ 负载均衡方差: " << std::fixed << std::setprecision(6)
                << variance << "\n";

     
        }
    }

    // 显示最佳解决方案统计
    if (solution_count > 0) {
        int best_solution = 0;
        double best_coverage = 0.0;

        for (int i = 0; i < solution_count; ++i) {
            int sat_count = 0;
            double cov = 0.0, var = 0.0;
            if (GetSolutionInfo(scheduler, i, &sat_count, &cov, &var)) {
                if (cov > best_coverage) {
                    best_coverage = cov;
                    best_solution = i;
                }
            }
        }

        std::cout << "[最佳解决方案]\n";
        std::cout << "  选择方案 #" << best_solution + 1
            << " (覆盖率最高" << ")\n";
    }
}

int main() {
    ShowWelcome();

    try {
        // 生成测试数据（如果需要）
        GenerateTestDataIfNeeded();

        // 1. 设置算法参数
        std::cout << "[初始化] 设置算法参数...\n";
        AlgorithmParams params;
        params.max_switches = 15;   //卫星在观测过程中的最大切换次数限制
        params.q = 6665;            //卫星网格中小格子总数量
        params.weights = { 0.34, 0.33, 0.33 };   //多目标优化中的权重分配
        params.max_sparks = 30;         //遗传算法中种群大小
        params.max_length = 10;         //探索序列长度限制
        params.update_bounds = true;
        params.max_variation = 3;       //遗传算法中控制变异的幅度或强度
        params.special_times = { 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 449 };
        //特殊时间点
        params.satellites = { 2, 4, 5 };   //需要特殊处理的卫星编号
        params.num_satellites_ = 10;        //卫星总数量
        params.total_minutes_ = 720;        //总的调度分钟数
        params.starthour = 8;
        params.startminute = 0;
        params.startsecond = 0;
        params.endhour = 20;
        params.endminute = 0;
        params.endsecond = 0;
        params.directoryPath = "";  // 使用当前目录
        params.data_dir_ = "";

        std::cout << "[参数] 卫星数量: " << params.num_satellites_ << "\n";
        std::cout << "[参数] 观测时间: " << params.starthour << ":00 - "
            << params.endhour << ":00 (" << params.total_minutes_ << "分钟)\n";
        std::cout << "[参数] 最大切换次数: " << params.max_switches << "\n";
        std::cout << "[参数] 网格分辨率(q): " << params.q << "\n";

        // 2. 创建调度器
        std::cout << "\n[调度器] 创建调度器实例...\n";
        void* scheduler = CreateScheduler(&params);
        if (!scheduler) {
            std::cerr << "\n错误: 创建调度器失败!\n";
            std::cerr << "错误信息: " << GetSchedulerLastError() << std::endl;
            return 1;
        }
        std::cout << "[调度器] 创建成功\n";

        // 3. 执行调度优化
        std::cout << "\n[优化] 开始执行调度优化...\n";
        std::cout << "[优化] 迭代次数: 50\n";
        std::cout << "[优化] 种群大小: 30\n";
        std::cout << "[优化] 随机种子: 12345\n";

        bool success = ExecuteScheduling(
            scheduler,
            "compressed_data.txt",  // 输出文件
            10,     // 迭代次数（调试时减少次数）
            30,     // 种群大小
            12345,  // 随机种子
            params.starthour,
            params.startminute,
            params.startsecond,
            params.endhour,
            params.endminute,
            params.endsecond
        );

        if (!success) {
            std::cerr << "\n错误: 调度优化失败!\n";
            std::cerr << "错误信息: " << GetSchedulerLastError() << std::endl;
            DestroyScheduler(scheduler);
            return 1;
        }

        std::cout << "\n[优化] 调度优化完成!\n";

        // 4. 显示结果
        DisplayResults(scheduler);

        // 5. 清理资源
        std::cout << "\n[清理] 释放调度器资源...\n";
        DestroyScheduler(scheduler);

        std::cout << "\n==================================================\n";
        std::cout << "程序执行成功完成!\n";
        std::cout << "==================================================\n\n";

        // 等待用户输入（调试时保持控制台打开）
        std::cout << "按Enter键退出程序...";
        std::cin.get();

    }
    catch (const std::exception& e) {
        std::cerr << "\n==================================================\n";
        std::cerr << "程序异常: " << e.what() << std::endl;
        std::cerr << "==================================================\n";
        std::cout << "\n按Enter键退出...";
        std::cin.get();
        return 1;
    }
    catch (...) {
        std::cerr << "\n==================================================\n";
        std::cerr << "未知异常" << std::endl;
        std::cerr << "==================================================\n";
        std::cout << "\n按Enter键退出...";
        std::cin.get();
        return 1;
    }

    return 0;
}