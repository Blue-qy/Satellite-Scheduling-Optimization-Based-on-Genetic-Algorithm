// pch.h - 预编译头文件
#pragma once

#ifndef PCH_H
#define PCH_H

// 添加要在此处预编译的标头
#include "framework.h"

// 标准库头文件
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <unordered_map>
#include <tuple>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <ctime>
#include <random>
#include <numeric>
#include <iomanip>
#include <filesystem>
#include <chrono>

// Windows API 头文件
#include <windows.h>

// 项目头文件
#include "satellite_scheduler_api.h"
#include "satellite_data_loader.h"
#include "coverage_loader.h"
#include "csv_parser.h"
#include "csv_reader_optimized.h"
#include "satellite_scheduler_base.h"
#include "satellite_scheduler_solution.h"
#include "satellite_scheduler_genetic.h"
#include "satellite_scheduler_multiobjective.h"

#endif //PCH_H