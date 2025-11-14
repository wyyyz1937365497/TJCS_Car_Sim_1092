#include "Class.h"
#include <memory>

// 小轿车类
struct Sedan : public Vehicle {
    Sedan(int lane, int carlength, int carwidth, int x, int y, int speed);
    // 重写变道函数，实现更快的变道曲线
    bool smoothLaneChange(int laneHeight, const std::vector<std::unique_ptr<Vehicle>> &allVehicles) override;
    // 获取小轿车的安全距离
    int getSafeDistance() const override;
    // 重写绘制函数
    void draw() const override;
};

// SUV类
struct SUV : public Vehicle {
    SUV(int lane, int carlength, int carwidth, int x, int y, int speed);
    // 重写变道函数，实现中等的变道曲线
    bool smoothLaneChange(int laneHeight, const std::vector<std::unique_ptr<Vehicle>> &allVehicles) override;
    // 获取SUV的安全距离
    int getSafeDistance() const override;
    // 重写绘制函数
    void draw() const override;
};

// 大卡车类
struct Truck : public Vehicle {
    Truck(int lane, int carlength, int carwidth, int x, int y, int speed);
    // 重写变道函数，实现更慢的变道曲线
    bool smoothLaneChange(int laneHeight, const std::vector<std::unique_ptr<Vehicle>> &allVehicles) override;
    // 获取大卡车的安全距离
    int getSafeDistance() const override;
    // 重写绘制函数
    void draw() const override;
};

#pragma once
