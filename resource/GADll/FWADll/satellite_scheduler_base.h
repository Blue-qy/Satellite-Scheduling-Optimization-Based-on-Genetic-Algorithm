#pragma once
// satellite_scheduler_base.h
#pragma once
#include <vector>
#include <string>
#include <memory>
#include <random>
#include "coverage_loader.h"
#include "satellite_scheduler_api.h"


class SatelliteSchedulerBase {
protected:
    std::vector<std::vector<int>> compressed_;
    std::vector<int> bounds_;
    int m_;

    // 算法参数 - 现在通过构造函数设置
    AlgorithmParams params_;

    std::unique_ptr<CoverageDataLoader> coverage_loader_;
    std::random_device rd_;
    std::mt19937 gen_;

public:
    // 修改构造函数以接受参数
    SatelliteSchedulerBase(const AlgorithmParams& params = AlgorithmParams());
    virtual ~SatelliteSchedulerBase() = default;

    bool loadCompressedData(const std::string& filename);
    void initializeCoverageLoader();

    const std::vector<std::vector<int>>& getCompressed() const { return compressed_; }
    const std::vector<int>& getBounds() const { return bounds_; }
    int getSatelliteCount() const { return m_; }

    // 获取和设置参数的函数
    const AlgorithmParams& getParams() const { return params_; }
    void setParams(const AlgorithmParams& params) { params_ = params; }

protected:
    std::vector<std::vector<int>> parseCompressedData(const std::vector<std::string>& lines);
    std::vector<int> parseBoundsData(const std::string& bounds_line);
};
