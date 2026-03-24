#pragma once
#include "satellite_scheduler_solution.h"
#include <vector>

class SatelliteSchedulerGenetic : public SatelliteSchedulerSolution {
public:
    SatelliteSchedulerGenetic(const AlgorithmParams& params = AlgorithmParams());

    // 遗传操作
    std::vector<std::vector<std::vector<int>>> selection(
        const std::vector<std::vector<std::vector<int>>>& population);

    std::vector<std::vector<std::vector<int>>> crossover(
        const std::vector<std::vector<int>>& parent1,
        const std::vector<std::vector<int>>& parent2);

    std::vector<std::vector<int>> mutate(
        const std::vector<std::vector<int>>& individual,
        double mutation_rate);

    std::vector<std::vector<std::vector<int>>> createNewGeneration(
        const std::vector<std::vector<std::vector<int>>>& population,
        int population_size);

    // 随机数辅助函数（继承或重新实现）
    double randomDouble(double min = 0.0, double max = 1.0);
    int randomInt(int min, int max);
    bool randomBool(double probability = 0.5);
    int randomSatellite();

private:
    // 辅助函数
    double calculateFitness(const std::vector<std::vector<int>>& solution);
    int tournamentSelection(const std::vector<double>& fitness_values);
    std::vector<int> crossoverSchedule(const std::vector<int>& parent1,
        const std::vector<int>& parent2);

    // 遗传算法参数
    double crossover_rate_;
    double mutation_rate_;
    int tournament_size_;
};