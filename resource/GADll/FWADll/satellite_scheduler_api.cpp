// simple_dll_test.cpp - 最简DLL调用测试
#include <iostream>
#include <windows.h>

// DLL函数指针定义
typedef void* (*CreateSchedulerFunc)(const void* params);
typedef bool (*ExecuteSchedulingFunc)(void* scheduler, int iterations, int population_size);
typedef const char* (*GetSchedulerLastErrorFunc)();
typedef void (*DestroySchedulerFunc)(void* scheduler);

// 参数结构体（必须与DLL中完全一致）
struct TestParams {
    int max_switches = 15;
    int q = 6665;
    // 注意：这里只保留DLL实际使用的字段，按声明顺序
    int num_satellites_ = 10;
    int total_minutes_ = 720;
    int starthour = 8;
    int startminute = 0;
    int startsecond = 0;
    int endhour = 20;
    int endminute = 0;
    int endsecond = 0;
    // 其他字段可能不需要，但为了安全保留基本字段
};

int main() {
    std::cout << "FWADll 简单调用测试" << std::endl;
    std::cout << "参数: q=" << 6665 << ", satellites=" << 10 << ", minutes=" << 720 << std::endl;

    // 加载DLL
    HMODULE hDll = LoadLibrary(L"FWADll.dll");
    if (!hDll) {
        std::cerr << "错误: 无法加载DLL" << std::endl;
        return -1;
    }

    // 获取函数地址
    auto createScheduler = (CreateSchedulerFunc)GetProcAddress(hDll, "CreateScheduler");
    auto executeScheduling = (ExecuteSchedulingFunc)GetProcAddress(hDll, "ExecuteScheduling");
    auto getLastError = (GetSchedulerLastErrorFunc)GetProcAddress(hDll, "GetSchedulerLastError");
    auto destroyScheduler = (DestroySchedulerFunc)GetProcAddress(hDll, "DestroyScheduler");

    if (!createScheduler || !executeScheduling || !getLastError || !destroyScheduler) {
        std::cerr << "错误: 获取函数地址失败" << std::endl;
        FreeLibrary(hDll);
        return -1;
    }

    // 设置参数
    TestParams params;
    params.q = 6665;
    params.num_satellites_ = 10;
    params.total_minutes_ = 720;
    params.starthour = 8;
    params.startminute = 0;
    params.startsecond = 0;
    params.endhour = 20;
    params.endminute = 0;
    params.endsecond = 0;
    params.max_switches = 15;

    std::cout << "正在创建调度器..." << std::endl;
    void* scheduler = createScheduler(&params);
    if (!scheduler) {
        const char* error = getLastError();
        std::cerr << "创建调度器失败: " << (error ? error : "未知错误") << std::endl;
        FreeLibrary(hDll);
        return -1;
    }
    std::cout << "调度器创建成功" << std::endl;

    std::cout << "正在执行调度..." << std::endl;
    bool result = executeScheduling(scheduler, 1, 1); // 最小迭代和种群
    if (!result) {
        const char* error = getLastError();
        std::cerr << "执行失败: " << (error ? error : "未知错误") << std::endl;
    }
    else {
        std::cout << "执行成功！" << std::endl;
    }

    // 清理
    destroyScheduler(scheduler);
    FreeLibrary(hDll);

    std::cout << "测试完成" << std::endl;
    return 0;
}