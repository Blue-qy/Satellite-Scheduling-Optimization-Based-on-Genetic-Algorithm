// pch.h - 预编译头文件
#pragma once

/*
dllmain.cpp: DLL入口点
Windows DLL的标准入口函数
处理DLL的加载和卸载
*/

// 添加常用的标准库头文件
#include "pch.h"
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <tuple>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <ctime>
#include <random>

// Windows API 头文件（如果需要）
#include <windows.h>

// 你的项目头文件
#include "satellite_data_loader.h"
#include "coverage_loader.h"
#include "satellite_scheduler_base.h"
#include "satellite_scheduler_solution.h"
#include "satellite_scheduler_multiobjective.h"
#include "satellite_scheduler_api.h"