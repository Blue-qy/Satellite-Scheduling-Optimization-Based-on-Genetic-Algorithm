// satellite_scheduler_api.h - 修改为DLL导出版本
#pragma once

#include <string>
#include <vector>

#ifdef SATELLITESCHEDULER_EXPORTS
#define SATELLITE_API __declspec(dllexport)
#else
#define SATELLITE_API __declspec(dllimport)
#endif

// 算法参数结构体
struct AlgorithmParams {
    int max_switches = 5;
    int q = 6665;
    std::vector<double> weights = { 0.34, 0.33, 0.33 };
    int max_sparks = 30;
    int max_length = 10;
    bool update_bounds = true;
    int max_variation = 3;
    std::vector<int> special_times = { 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 449 };
    std::vector<int> satellites = { 2, 4, 5 };
    int num_satellites_ = 10;
    int total_minutes_ = 721;
    int starthour = 8;
    int startminute = 0;
    int startsecond = 0;
    int endhour = 20;
    int endminute = 0;
    int endsecond = 0;
    std::string directoryPath = "";
    std::string data_dir_ = "";

    // 默认构造函数
    AlgorithmParams() = default;

    // 带参数的构造函数
    AlgorithmParams(int max_sw, int q_val, const std::vector<double>& w,
        int max_sp, int max_len, bool update_b, int max_var,
        const std::vector<int>& special_t)
        : max_switches(max_sw), q(q_val), weights(w), max_sparks(max_sp),
        max_length(max_len), update_bounds(update_b), max_variation(max_var),
        special_times(special_t) {}
};

// API函数声明
extern "C" {
    SATELLITE_API void* CreateScheduler(const AlgorithmParams* params);
    SATELLITE_API void DestroyScheduler(void* scheduler);
    SATELLITE_API bool ExecuteScheduling(
        void* scheduler,
        const char* compressed_data_file,
        int num_iterations,
        int population_size,
        int random_seed,
        int start_hour,
        int start_minute,
        int start_second,
        int end_hour,
        int end_minute,
        int end_second
    );
    SATELLITE_API const char* GetSchedulerLastError();

    // 添加获取结果的函数
    SATELLITE_API int GetSolutionCount(void* scheduler);
    SATELLITE_API bool GetSolutionInfo(void* scheduler, int solution_index,
        int* satellite_count, double* coverage, double* variance);
    SATELLITE_API int GetAssignmentCount(void* scheduler, int solution_index, int satellite_index);
    SATELLITE_API int GetAssignment(void* scheduler, int solution_index, int satellite_index, int task_index);
}