#pragma once
#include <vector>
#include <string>
#include <fstream>
#include "Class.h"

// 车辆统计数据记录类
class VehicleStatistics
{
public:
    // 车辆出现计数
    struct VehicleCount
    {
        int sedanCount;
        int suvCount;
        int truckCount;
        int totalCount;

        VehicleCount() : sedanCount(0), suvCount(0), truckCount(0), totalCount(0) {}
    };

    // 参数记录结构
    struct ParameterRecord
    {
        double time;          // 运行时间
        int safeDistance;     // 探测距离
        int stoppingSpeed;    // 减速度
        int breakdownCount;   // 抛锚车辆数
        int vehicleCount;     // 总车辆数
        double breakdownRate; // 故障率 (breakdownCount/vehicleCount)

        ParameterRecord() : time(0), safeDistance(0), stoppingSpeed(0),
                            breakdownCount(0), vehicleCount(0), breakdownRate(0.0) {}
    };

private:
    VehicleCount vehicleCount;
    std::vector<ParameterRecord> parameterRecords;
    int lastSafeDistance;
    int lastStoppingSpeed;
    double currentTime;

public:
    VehicleStatistics();

    // 记录生成的车辆类型
    void recordVehicle(const Vehicle *vehicle);

    // 检查参数是否改变并记录
    void checkAndRecordParameters(double time, int safeDist, int stopSpeed,
                                  const std::vector<Vehicle *> &vehicles);

    // 保存车辆出现概率统计到CSV文件
    void saveVehicleProbabilityStatistics() const;

    // 保存参数变化时的故障率统计到CSV文件
    void saveBreakdownRateStatistics() const;

    // 程序结束时保存所有统计数据
    void saveAllStatistics() const;
};