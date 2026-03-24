// satellite_scheduler_multiobjective.cpp

#include "pch.h"
#include "satellite_scheduler_multiobjective.h"

#include <algorithm>
#include <limits>
#include <iostream>
#include <numeric>

SatelliteSchedulerMultiObjective::SatelliteSchedulerMultiObjective(const AlgorithmParams& params)
    : SatelliteSchedulerGenetic(params) {
}

std::unordered_map<int, std::vector<std::vector<std::vector<int>>>>
SatelliteSchedulerMultiObjective::fastNonDominatedSort(
    const std::vector<std::vector<std::vector<int>>>& solutions) {

    std::unordered_map<int, std::vector<std::vector<std::vector<int>>>> fronts;

    if (solutions.empty()) {
        return fronts;
    }

    // 初始化数据结构
    std::vector<int> domination_count(solutions.size(), 0);
    std::vector<std::vector<int>> dominated_set(solutions.size());

    // 计算所有解的目标值
    std::vector<std::vector<double>> objectives;
    objectives.reserve(solutions.size());

    for (const auto& sol : solutions) {
        auto eval_result = evaluate(sol, false);
        objectives.push_back({
            static_cast<double>(eval_result.satellite_count),    // f1: 最小化
            -eval_result.coverage,          // f2: 最大化 -> 取负最小化
            eval_result.load_variance       // f3: 最小化
            });
    }

    // 计算支配关系
    for (size_t i = 0; i < solutions.size(); ++i) {
        for (size_t j = 0; j < solutions.size(); ++j) {
            if (i == j) continue;

            if (dominates(objectives[i], objectives[j])) {
                dominated_set[i].push_back(static_cast<int>(j));
            }
            else if (dominates(objectives[j], objectives[i])) {
                domination_count[i]++;
            }
        }

        if (domination_count[i] == 0) {
            // 添加到第一前沿
            if (fronts.find(1) == fronts.end()) {
                fronts[1] = {};
            }
            fronts[1].push_back(solutions[i]);
        }
    }

    // 构建后续前沿
    int current_rank = 1;
    while (fronts.find(current_rank) != fronts.end() && !fronts[current_rank].empty()) {
        std::vector<std::vector<std::vector<int>>> next_front;

        for (const auto& sol : fronts[current_rank]) {
            // 找到当前解在原始列表中的索引
            auto it = std::find(solutions.begin(), solutions.end(), sol);
            if (it != solutions.end()) {
                int i = static_cast<int>(std::distance(solutions.begin(), it));

                for (int j : dominated_set[i]) {
                    domination_count[j]--;
                    if (domination_count[j] == 0) {
                        next_front.push_back(solutions[j]);
                    }
                }
            }
        }

        current_rank++;
        if (!next_front.empty()) {
            fronts[current_rank] = next_front;
        }
        else {
            break;
        }
    }

    return fronts;
}

std::vector<std::vector<std::vector<int>>>
SatelliteSchedulerMultiObjective::crowdingSelection(
    const std::vector<std::vector<std::vector<int>>>& solutions,
    int select_num) {

    if (solutions.size() <= select_num) {
        return solutions;
    }

    // 将解决方案包装为带有目标值的结构
    std::vector<SolutionWithObjectives> sols_with_obj;
    sols_with_obj.reserve(solutions.size());

    for (const auto& sol : solutions) {
        SolutionWithObjectives sol_obj;
        sol_obj.solution = sol;
        sol_obj.objectives = solutionToObjectives(sol);
        sol_obj.crowding_distance = 0.0;
        sols_with_obj.push_back(sol_obj);
    }

    // 对每个目标分别计算拥挤距离
    for (int m = 0; m < 3; ++m) {
        // 按目标m排序
        std::sort(sols_with_obj.begin(), sols_with_obj.end(),
            [m](const SolutionWithObjectives& a, const SolutionWithObjectives& b) {
                return a.objectives[m] < b.objectives[m];
            });

        // 设置边界解的拥挤距离为无穷大
        if (!sols_with_obj.empty()) {
            sols_with_obj.front().crowding_distance = std::numeric_limits<double>::infinity();
            sols_with_obj.back().crowding_distance = std::numeric_limits<double>::infinity();
        }

        // 计算中间解的拥挤距离
        if (sols_with_obj.size() > 2) {
            double min_obj = sols_with_obj.front().objectives[m];
            double max_obj = sols_with_obj.back().objectives[m];
            double range = max_obj - min_obj;

            if (range > std::numeric_limits<double>::epsilon()) {
                for (size_t i = 1; i < sols_with_obj.size() - 1; ++i) {
                    double distance = (sols_with_obj[i + 1].objectives[m] - sols_with_obj[i - 1].objectives[m]) / range;
                    sols_with_obj[i].crowding_distance += distance;
                }
            }
        }
    }

    // 按拥挤距离排序（降序）
    std::sort(sols_with_obj.begin(), sols_with_obj.end(),
        [](const SolutionWithObjectives& a, const SolutionWithObjectives& b) {
            return a.crowding_distance > b.crowding_distance;
        });

    // 选择前select_num个解
    std::vector<std::vector<std::vector<int>>> selected;
    selected.reserve(select_num);

    for (int i = 0; i < select_num && i < static_cast<int>(sols_with_obj.size()); ++i) {
        selected.push_back(sols_with_obj[i].solution);
    }

    return selected;
}

bool SatelliteSchedulerMultiObjective::dominates(
    const std::vector<double>& obj_a, const std::vector<double>& obj_b) {

    if (obj_a.size() != 3 || obj_b.size() != 3) {
        return false;
    }

    // 所有目标都不差于b（三个目标都是最小化）
    bool not_worse = (obj_a[0] <= obj_b[0]) &&
        (obj_a[1] <= obj_b[1]) &&
        (obj_a[2] <= obj_b[2]);

    // 至少有一个目标严格优于b
    bool better_in_one = (obj_a[0] < obj_b[0]) ||
        (obj_a[1] < obj_b[1]) ||
        (obj_a[2] < obj_b[2]);

    return not_worse && better_in_one;
}

std::pair<std::vector<std::vector<std::vector<int>>>,
    std::vector<std::vector<std::vector<int>>>>
    SatelliteSchedulerMultiObjective::optimize(int max_iter, int num_fireworks, int max_changes) {

    // 使用遗传算法术语重命名参数
    int population_size = num_fireworks;  // 种群大小
    int max_generations = max_iter;       // 最大代数

    // 1. 初始化种群
    std::vector<std::vector<std::vector<int>>> population;
    population.reserve(population_size);

    for (int i = 0; i < population_size; ++i) {
        auto solution = initializeSolution();
        // 确保满足切换约束
        if (checkSwitches(solution)) {
            population.push_back(solution);
        }
        else {
            // 如果不满足约束，重新生成
            i--;
        }
    }

    std::vector<std::vector<std::vector<int>>> all_candidates = population;
    all_candidates.reserve(population_size * (max_generations + 1));

    std::cout << "开始遗传算法优化，迭代次数: " << max_generations
        << ", 种群大小: " << population_size << std::endl;

    // 2. 迭代优化
    for (int generation = 0; generation < max_generations; ++generation) {
        // 创建新一代
        population = createNewGeneration(population, population_size);

        // 合并到候选解集
        all_candidates.insert(all_candidates.end(), population.begin(), population.end());

        // 快速非支配排序获取当前前沿
        auto ranked = fastNonDominatedSort(population);

        // 输出进度信息
        if (generation % 1 == 0 || generation == max_generations - 1) {
            std::cout << "迭代 " << generation + 1 << "/" << max_generations;

            if (ranked.find(1) != ranked.end() && !ranked[1].empty()) {
                auto best = ranked[1][0];
                auto best_eval = evaluate(best, false);

                // 计算每颗卫星的平均负载
                double avg_load = 0.0;
                int active_satellites = 0;
                for (const auto& schedule : best) {
                    int load = std::count_if(schedule.begin(), schedule.end(),
                        [](int val) { return val == 1; });
                    if (load > 0) {
                        avg_load += load;
                        active_satellites++;
                    }
                }
                if (active_satellites > 0) {
                    avg_load /= active_satellites;
                }

                std::cout << " - 最佳解: 卫星=" << best_eval.satellite_count
                    << ", 覆盖率时间积=" << std::fixed << std::setprecision(2)
                    << best_eval.coverage << " (平均每时间片覆盖率：" << best_eval.coverage / bounds_.size() << ")"
                    << ", 方差=" << best_eval.load_variance
                    << ", 平均负载=" << avg_load;
            }
            std::cout << std::endl;
        }

        // 提前终止条件（可选）
        if (generation > 50 && ranked.find(1) != ranked.end() && ranked[1].size() >= 5) {
            // 检查最近几代是否没有改进
            if (generation % 20 == 0) {
                static double last_best_coverage = 0.0;
                auto current_best = ranked[1][0];
                auto current_eval = evaluate(current_best, false);

                if (std::abs(current_eval.coverage - last_best_coverage) < 0.01) {
                    std::cout << "收敛条件满足，提前终止" << std::endl;
                    break;
                }
                last_best_coverage = current_eval.coverage;
            }
        }
    }

    // 3. 最终排序和返回
    auto final_ranking = fastNonDominatedSort(population);
    auto all_ranking = fastNonDominatedSort(all_candidates);

    std::vector<std::vector<std::vector<int>>> best_solutions;
    if (final_ranking.find(1) != final_ranking.end()) {
        best_solutions = final_ranking[1];
    }

    std::vector<std::vector<std::vector<int>>> all_solutions;
    if (all_ranking.find(1) != all_ranking.end()) {
        all_solutions = all_ranking[1];
    }

    std::cout << "优化完成，找到 " << best_solutions.size() << " 个最优解" << std::endl;
    std::cout << "候选解总数: " << all_candidates.size() << std::endl;

    return { best_solutions, all_solutions };
}

std::vector<double> SatelliteSchedulerMultiObjective::solutionToObjectives(
    const std::vector<std::vector<int>>& solution) {

    auto eval_result = evaluate(solution, false);
    return {
        static_cast<double>(eval_result.satellite_count),    // f1: 最小化
        -eval_result.coverage,          // f2: 最大化 -> 取负最小化  
        eval_result.load_variance       // f3: 最小化
    };
}

double SatelliteSchedulerMultiObjective::calculateCrowdingDistance(
    const std::vector<SolutionWithObjectives>& solutions,
    int solution_idx, int objective_idx) {

    if (solution_idx == 0 || solution_idx == static_cast<int>(solutions.size()) - 1) {
        return std::numeric_limits<double>::infinity();
    }

    if (solutions.size() < 3) {
        return 1.0;
    }

    double prev_obj = solutions[solution_idx - 1].objectives[objective_idx];
    double next_obj = solutions[solution_idx + 1].objectives[objective_idx];

    // 计算目标值范围
    double min_obj = solutions.front().objectives[objective_idx];
    double max_obj = solutions.back().objectives[objective_idx];
    double range = max_obj - min_obj;

    if (range < std::numeric_limits<double>::epsilon()) {
        return 1.0;
    }

    return (next_obj - prev_obj) / range;
}