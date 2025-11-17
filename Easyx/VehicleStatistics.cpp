#include "VehicleStatistics.h"
#include "Class.h"
#include "VehicleTypes.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <direct.h>  // For _mkdir on Windows

VehicleStatistics::VehicleStatistics() : lastSafeDistance(-1), lastStoppingSpeed(-1), 
                                       lastVehicleCount(-1), lastBreakdownCount(-1), currentTime(0.0)
{
}

void VehicleStatistics::recordVehicle(const Vehicle *vehicle)
{
    // 根据车辆类型增加计数
    if (dynamic_cast<const class Sedan *>(vehicle))
    {
        vehicleCount.sedanCount++;
    }
    else if (dynamic_cast<const class SUV *>(vehicle))
    {
        vehicleCount.suvCount++;
    }
    else if (dynamic_cast<const class Truck *>(vehicle))
    {
        vehicleCount.truckCount++;
    }

    vehicleCount.totalCount++;
}

void VehicleStatistics::checkAndRecordParameters(double time, int safeDist, int stopSpeed,
                                                 const std::vector<Vehicle *> &vehicles)
{
    currentTime = time;

    // 检查参数是否发生变化或者车辆数量发生变化（考虑车辆被删除的情况）
    int breakdownCount = 0;
    for (const auto &vehicle : vehicles)
    {
        if (vehicle->isBrokenDown)
        {
            breakdownCount++;
        }
    }
    
    // 只有当关键参数发生变化时才记录，时间和车辆总数变化不应导致记录中断
    // 关键参数包括：安全距离、减速度
    // 其他参数（如车辆总数、故障车辆数）的变化不影响记录的连续性
    if (safeDist != lastSafeDistance || stopSpeed != lastStoppingSpeed)
    {
        // 记录当前参数
        lastSafeDistance = safeDist;
        lastStoppingSpeed = stopSpeed;
        lastVehicleCount = static_cast<int>(vehicles.size());
        lastBreakdownCount = breakdownCount;

        // 创建新的记录
        ParameterRecord record;
        record.time = time;
        record.safeDistance = safeDist;
        record.stoppingSpeed = stopSpeed;
        record.breakdownCount = breakdownCount;
        record.vehicleCount = static_cast<int>(vehicles.size());
        record.breakdownRate = (vehicles.size() > 0) ? static_cast<double>(breakdownCount) / vehicles.size() : 0.0;

        parameterRecords.push_back(record);
    }
    // 如果仅仅是时间或车辆数量变化，不创建新的记录条目，保持当前记录连续性
}

void VehicleStatistics::saveVehicleProbabilityStatistics() const
{
    // 确保log目录存在
    _mkdir("./log");
    
    std::ofstream file("./log/vehicle_probability_statistics.csv");
    if (!file.is_open())
    {
        std::cerr << "无法创建车辆概率统计文件!" << std::endl;
        return;
    }

    // 写入CSV头部
    file << "VehicleType,Count,Probability\n";

    // 写入各车辆类型的统计数据
    if (vehicleCount.totalCount > 0)
    {
        file << "Sedan," << vehicleCount.sedanCount << ","
             << std::fixed << std::setprecision(4)
             << static_cast<double>(vehicleCount.sedanCount) / vehicleCount.totalCount << "\n";
        file << "SUV," << vehicleCount.suvCount << ","
             << std::fixed << std::setprecision(4)
             << static_cast<double>(vehicleCount.suvCount) / vehicleCount.totalCount << "\n";
        file << "Truck," << vehicleCount.truckCount << ","
             << std::fixed << std::setprecision(4)
             << static_cast<double>(vehicleCount.truckCount) / vehicleCount.totalCount << "\n";
    }
    else
    {
        file << "Sedan,0,0.0000\n";
        file << "SUV,0,0.0000\n";
        file << "Truck,0,0.0000\n";
    }

    file.close();
}

void VehicleStatistics::saveBreakdownRateStatistics() const
{
    // 确保log目录存在
    _mkdir("./log");
    
    std::ofstream file("./log/breakdown_rate_statistics.csv");
    if (!file.is_open())
    {
        std::cerr << "无法创建故障率统计文件!" << std::endl;
        return;
    }

    // 写入CSV头部
    file << "Time(s),SafeDistance,Deceleration,BreakdownCount,VehicleCount,BreakdownRate\n";

    // 写入参数记录数据
    for (const auto &record : parameterRecords)
    {
        file << std::fixed << std::setprecision(1) << record.time << ","
             << record.safeDistance << ","
             << record.stoppingSpeed << ","
             << record.breakdownCount << ","
             << record.vehicleCount << ","
             << std::fixed << std::setprecision(4) << record.breakdownRate << "\n";
    }

    file.close();
}

void VehicleStatistics::saveAllStatistics() const
{
    saveVehicleProbabilityStatistics();
    saveBreakdownRateStatistics();
}