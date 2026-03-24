#include "pch.h"
#include "satellite_scheduler_genetic.h"
#include <algorithm>
#include <numeric>

SatelliteSchedulerGenetic::SatelliteSchedulerGenetic(const AlgorithmParams& params)
    : SatelliteSchedulerSolution(params),
    crossover_rate_(0.8),
    mutation_rate_(0.1),
    tournament_size_(3) {
}

// 实现随机数辅助函数
double SatelliteSchedulerGenetic::randomDouble(double min, double max) {
    std::uniform_real_distribution<double> dist(min, max);
    return dist(gen_);  // gen_ 从基类继承而来
}

int SatelliteSchedulerGenetic::randomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen_);
}

bool SatelliteSchedulerGenetic::randomBool(double probability) {
    std::bernoulli_distribution dist(probability);
    return dist(gen_);
}

int SatelliteSchedulerGenetic::randomSatellite() {
    return randomInt(0, m_ - 1);
}

double SatelliteSchedulerGenetic::calculateFitness(
    const std::vector<std::vector<int>>& solution) {

    auto eval_result = evaluate(solution, true);
    // 加权适应度（越大越好）
    return params_.weights[1] * eval_result.coverage -  // 覆盖率（最大化）
        params_.weights[0] * eval_result.satellite_count -  // 卫星数（最小化）
        params_.weights[2] * eval_result.load_variance;  // 方差（最小化）
}

std::vector<std::vector<std::vector<int>>>
SatelliteSchedulerGenetic::selection(
    const std::vector<std::vector<std::vector<int>>>& population) {

    // 计算所有个体的适应度
    std::vector<double> fitness_values;
    fitness_values.reserve(population.size());
    for (const auto& individual : population) {
        fitness_values.push_back(calculateFitness(individual));
    }

    // 锦标赛选择
    std::vector<std::vector<std::vector<int>>> selected;
    selected.reserve(population.size());

    for (size_t i = 0; i < population.size(); ++i) {
        int best_idx = tournamentSelection(fitness_values);
        selected.push_back(population[best_idx]);
    }

    return selected;
}

int SatelliteSchedulerGenetic::tournamentSelection(
    const std::vector<double>& fitness_values) {

    int best_idx = -1;
    double best_fitness = -std::numeric_limits<double>::infinity();

    // 随机选择tournament_size_个个体进行锦标赛
    for (int i = 0; i < tournament_size_; ++i) {
        int random_idx = randomInt(0, static_cast<int>(fitness_values.size() - 1));
        if (fitness_values[random_idx] > best_fitness) {
            best_fitness = fitness_values[random_idx];
            best_idx = random_idx;
        }
    }

    return best_idx;
}

std::vector<std::vector<std::vector<int>>>
SatelliteSchedulerGenetic::crossover(
    const std::vector<std::vector<int>>& parent1,
    const std::vector<std::vector<int>>& parent2) {

    if (randomDouble() > crossover_rate_) {
        // 不进行交叉，直接返回父代
        return { parent1, parent2 };
    }

    std::vector<std::vector<int>> child1;
    std::vector<std::vector<int>> child2;
    child1.reserve(m_);
    child2.reserve(m_);

    // 对每颗卫星的调度分别进行交叉
    for (int sat = 0; sat < m_; ++sat) {
        std::vector<int> child1_schedule = crossoverSchedule(parent1[sat], parent2[sat]);
        std::vector<int> child2_schedule = crossoverSchedule(parent2[sat], parent1[sat]);

        child1.push_back(std::move(child1_schedule));
        child2.push_back(std::move(child2_schedule));
    }

    return { std::move(child1), std::move(child2) };
}

std::vector<int> SatelliteSchedulerGenetic::crossoverSchedule(
    const std::vector<int>& parent1,
    const std::vector<int>& parent2) {

    std::vector<int> child(parent1.size(), -1);

    // 单点交叉
    if (parent1.size() > 1) {
        int crossover_point = randomInt(1, static_cast<int>(parent1.size() - 2));

        for (size_t i = 0; i < parent1.size(); ++i) {
            if (i < crossover_point) {
                child[i] = parent1[i];
            }
            else {
                child[i] = parent2[i];
            }
        }
    }
    else {
        // 如果只有一个元素，随机选择一个父代
        child = randomDouble() < 0.5 ? parent1 : parent2;
    }

    return child;
}

std::vector<std::vector<int>> SatelliteSchedulerGenetic::mutate(
    const std::vector<std::vector<int>>& individual,
    double mutation_rate) {

    std::vector<std::vector<int>> mutated = individual;

    for (int sat = 0; sat < m_; ++sat) {
        for (size_t time = 0; time < individual[sat].size(); ++time) {
            if (individual[sat][time] != -1 && randomDouble() < mutation_rate) {
                // 翻转位（0变1，1变0）
                mutated[sat][time] = 1 - individual[sat][time];
            }
        }
    }

    return mutated;
}

std::vector<std::vector<std::vector<int>>>
SatelliteSchedulerGenetic::createNewGeneration(
    const std::vector<std::vector<std::vector<int>>>& population,
    int population_size) {

    if (population.empty()) {
        return {};
    }

    std::vector<std::vector<std::vector<int>>> new_generation;
    new_generation.reserve(population_size);

    // 1. 选择
    auto selected = selection(population);

    // 2. 交叉和变异
    for (size_t i = 0; i + 1 < selected.size(); i += 2) {
        if (new_generation.size() >= population_size) {
            break;
        }

        auto children = crossover(selected[i], selected[i + 1]);

        for (auto& child : children) {
            // 变异
            child = mutate(child, mutation_rate_);

            // 确保满足约束
            if (checkSwitches(child)) {
                new_generation.push_back(std::move(child));
            }

            if (new_generation.size() >= population_size) {
                break;
            }
        }
    }

    // 3. 精英保留：保留当前代中的最优个体
    if (!population.empty() && !new_generation.empty()) {
        // 找到适应度最高的个体
        std::vector<double> fitness_values;
        fitness_values.reserve(population.size());

        for (const auto& individual : population) {
            fitness_values.push_back(calculateFitness(individual));
        }

        auto max_it = std::max_element(fitness_values.begin(), fitness_values.end());
        int elite_idx = static_cast<int>(std::distance(fitness_values.begin(), max_it));

        // 替换新代中的最差个体
        std::vector<double> new_fitness_values;
        new_fitness_values.reserve(new_generation.size());

        for (const auto& individual : new_generation) {
            new_fitness_values.push_back(calculateFitness(individual));
        }

        auto min_it = std::min_element(new_fitness_values.begin(), new_fitness_values.end());
        int worst_idx = static_cast<int>(std::distance(new_fitness_values.begin(), min_it));

        new_generation[worst_idx] = population[elite_idx];
    }

    // 4. 如果新代个体不足，用随机个体填充
    while (new_generation.size() < population_size) {
        new_generation.push_back(initializeSolution());
    }

    // 5. 确保大小正确
    if (new_generation.size() > population_size) {
        new_generation.resize(population_size);
    }

    return new_generation;
}