// test_correct_api.cpp
#include <iostream>
#include <cstring>
#include "satellite_scheduler_api.h"

int main() {
    std::cout << "烟花算法开始卫星调度测试..." << std::endl;

    // 初始化算法参数
    AlgorithmParams params;
    params.directoryPath = "";
    params.data_dir_ = "";

    params.num_satellites_ = 10;
    params.max_switches = 30;
    params.q = 6665;
    params.weights = { 0.34, 0.33, 0.33 };
    params.max_sparks = 30;

    params.satellites = { 2, 4, 5 };   //需要特殊处理的卫星编号
    params.max_length = 15;
    params.update_bounds = true;
    params.max_variation = 3;
    params.special_times = { 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 449 };
    params.starthour = 8;
    params.startminute = 0;
    params.startsecond = 0;
    params.endhour = 20;
    params.endminute = 0;
    params.endsecond = 0;

    int endSeconds = params.endhour * 3600 + params.endminute * 60 + params.endsecond;
    int startSeconds = params.starthour * 3600 + params.startminute * 60 + params.startsecond;
    int diffSeconds = endSeconds - startSeconds;
    int diffMinutes = diffSeconds / 60;
    params.total_minutes_ = diffMinutes + 1;

    // 创建调度器
    void* scheduler = CreateScheduler(&params);
    if (!scheduler) {
        std::cerr << "创建调度器失败: " << GetSchedulerLastError() << std::endl;
        return 1;
    }
    std::cout << "调度器创建成功!" << std::endl;

    // 执行调度（注意：你的API中ExecuteScheduling没有SchedulingResult参数）
    bool success = ExecuteScheduling(
        scheduler,
        "compressed_data.txt",
        10,30, 42,  // iterations=1, population_size=1, seed=42
        8, 0, 0,
        20, 0, 0
    );

    if (success) {
        std::cout << "调度成功完成!" << std::endl;

        int solution_count = GetSolutionCount(scheduler);
        std::cout << "找到 " << solution_count << " 个最优解" << std::endl;
    }
    else {
        std::cerr << "调度失败: " << GetSchedulerLastError() << std::endl;
    }

    // 清理资源
    DestroyScheduler(scheduler);
    std::cout << "程序执行完毕。" << std::endl;
    return 0;
}