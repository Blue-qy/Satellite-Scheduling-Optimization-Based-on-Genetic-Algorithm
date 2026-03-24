# Satellite Scheduling Optimization Based on Genetic Algorithm

基于遗传算法的卫星调度优化项目

## 项目结构说明

- **resource/**：编译生成 DLL 文件的源码目录
- **test_program/**：测试调用 DLL 的主程序目录，包含 main.cpp

## 使用方法

1. 打开 `resource` 文件夹中的项目文件
2. 在 Visual Studio 中**生成解决方案**，编译得到 DLL 文件
3. 将编译好的 **DLL 文件** 复制到 `test_program` 目录下
4. 运行 `test_program` 中的 `main.cpp` 即可调用 DLL 功能

## 项目结构

```cpp
GA/
├── x64/             # 构建输出目录
│   ├── Debug/       # 调试构建
│   └── Release/     # 发布构建
├── GA.vcxproj       # Visual Studio项目文件
├── GA.vcxproj.filters
├── GA.vcxproj.user
├── coverage_loader.cpp/h       # 覆盖加载器
├── csv_parser.cpp/h             # CSV文件解析器
├── csv_parser_optimized.cpp     # 优化的CSV解析器
├── csv_reader_optimized.h
├── dllmain.cpp                  # DLL入口
├── framework.h                  # 框架头文件
├── pch.cpp/h                    # 预编译头
├── satellite_data_loader.cpp/h  # 卫星数据加载器
├── satellite_schduler_multiobjective.cpp
├── satellite_scheduler_api.cpp/h     # 卫星调度API
├── satellite_scheduler_base.cpp/h    # 卫星调度基础
├── satellite_scheduler_fireworks.cpp/h  # 烟花算法调度
├── satellite_scheduler_multiobjective.h  # 多目标调度
└── satellite_scheduler_solution.cpp/h  # 调度解决方案
```
