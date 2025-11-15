#include <graphics.h>
#include <graphics.h>
#include <vector>
#include <ctime>
#include <conio.h> // 需要包含此头文件_kbhit()函数需要
#include <Windows.h>
#include <sstream>
#include <string>
#include <iostream>
#include <functional>
#include "Define.h"
using namespace std;
// 车辆类型枚举
enum class VehicleType
{
    SEDAN, // 小轿车
    SUV,   // SUV
    TRUCK  // 大卡车
};
int Sedan_curve(int t);
int SUV_curve(int t);
int Truck_curve(int t);
// 定义车辆的类
struct Vehicle
{
    int lane, carlength, carwidth, x, y, speed;
    bool haschanged;
    COLORREF color;

    // 默认构造函数
    Vehicle(int l = 0, int cl = 0, int cw = 0, int x = 0, int y = 0, int s = 0, bool hc = false, COLORREF c = RGB(255, 255, 255),
            bool icl = false, bool igc = false, int tl = 0, float cp = 0.0f,
            int sx = 0, int sy = 0, int ex = 0, int ey = 0, bool itc = false, COLORREF oc = RGB(255, 255, 255), bool ibd = false)
        : lane(l), carlength(cl), carwidth(cw), x(x), y(y), speed(s), haschanged(hc), color(c),
          isChangingLane(icl), isGoing2change(igc), targetLane(tl), changeProgress(cp),
          startX(sx), startY(sy), endX(ex), endY(ey), isTooClose(itc), originalColor(oc), isBrokenDown(ibd) {}

    // 新增成员变量用于变道
    bool isChangingLane; // 是否正在变道
    bool isGoing2change;
    int targetLane;       // 目标车道
    float changeProgress; // 变道进度 (0.0-1.0)
    int startX;           // 变道起始X坐标
    int startY;           // 变道起始Y坐标
    int endX;             // 变道结束X坐标
    int endY;             // 变道结束Y坐标
    // 预计算的变道轨迹点
    vector<pair<int, int>> laneChangeTrajectory; // 变道轨迹点集合

    // 距离警告相关成员
    bool isTooClose;        // 是否距离前车过近
    COLORREF originalColor; // 原始颜色

    // 抛锚状态
    bool isBrokenDown; // 车辆是否抛锚
    virtual void draw() const;
    // 预测并绘制轨迹
    void predictAndDrawTrajectory(int laneHeight, int middleY, int predictionSteps = 30, const vector<Vehicle *> &allVehicles = vector<Vehicle *>()) const;

    // 检查变道是否安全
    bool isLaneChangeSafe(int laneHeight, const vector<Vehicle *> &allVehicles) const;

    // 检查与前车距离
    void checkFrontVehicleDistance(vector<Vehicle *> &allVehicles,int safeDistance,int laneHeight);

    // 显示闪烁的橘色线框
    void showFlashingFrame();
    // 处理危险情况
    void handleDangerousSituation();
    // 前向运动函数
    void moveForward(int middleY)
    {
        x += (y < middleY) ? speed : -speed;
    }
    // 平滑变道函数
    virtual bool smoothLaneChange(int laneHeight, const vector<Vehicle *> &allVehicles);
    // 获取安全距离（可被子类重写）
    virtual int getSafeDistance() const;

    // 修改变道曲线函数为虚函数，允许子类重写
    virtual float curveFunc(float t) const { return 3 * t * t - 2 * t * t * t; }

    // 在车辆上方显示消息
    void carmessage(const string& message) const;
};

// 虚拟车辆类，用于轨迹预测和相交检测
struct VirtualVehicle
{
    int x, y;
    int carlength, carwidth;
    vector<pair<int, int>> trajectory; // 轨迹点集合

    VirtualVehicle(int startX, int startY, int length, int width)
        : x(startX), y(startY), carlength(length), carwidth(width) {}

    // 添加轨迹点
    void addTrajectoryPoint(int pointX, int pointY)
    {
        trajectory.push_back(make_pair(pointX, pointY));
    }

    // 绘制轨迹（根据安全情况使用不同颜色）
    void drawTrajectory(bool isSafe) const;
    // 检查与另一车辆的轨迹是否相交
    bool isTrajectoryIntersecting(const VirtualVehicle &other, int futureSteps) const;
};

// 定义桥的类
struct Bridge
{
    double bridgeLength, bridgeWidth, widthScale;
    // 采用此函数计算适合屏幕的窗口尺寸和缩放比例，准备绘制桥梁、车道
    // 根据屏幕分辨率调整窗口大小
    void calculateWindowSize(int &windowWidth, int &windowHeight, double &scale) const;
};
void clearLane(vector<Vehicle *> &vehicles, int lane);

#pragma once
