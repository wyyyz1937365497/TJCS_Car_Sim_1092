#pragma once

#include <QtWidgets/QWidget>
#include <QTimer>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <vector>
#include <random>
#include "ui_qtwidgetapplication.h"

// 声明前置类
struct Vehicle;
enum class TimeOfDay;

// 车辆类型枚举
enum class VehicleType
{
    SEDAN, // 小轿车
    SUV,   // SUV
    TRUCK  // 大卡车
};

// 天气模式枚举
enum WeatherMode
{
    RAIN,
    SNOW,
    NOTHING
};

// 时间段枚举
enum class TimeOfDay
{
    Day,
    Night
};

// 虚拟车辆类，用于轨迹预测和相交检测
struct VirtualVehicle
{
    int x, y;
    int carlength, carwidth;
    std::vector<std::pair<int, int>> trajectory; // 轨迹点集合

    VirtualVehicle(int startX, int startY, int length, int width)
        : x(startX), y(startY), carlength(length), carwidth(width) {}

    // 添加轨迹点
    void addTrajectoryPoint(int pointX, int pointY)
    {
        trajectory.push_back(std::make_pair(pointX, pointY));
    }
    
    // 检查两个虚拟车辆的轨迹是否相交
    bool isTrajectoryIntersecting(const VirtualVehicle& other, int predictionSteps) const
    {
        // 简化实现：检查轨迹点是否重叠
        for (const auto& point1 : trajectory) {
            for (const auto& point2 : other.trajectory) {
                // 检查两点间距离是否小于车辆宽度之和的一半（简化碰撞检测）
                int dx = point1.first - point2.first;
                int dy = point1.second - point2.second;
                int distance = sqrt(dx * dx + dy * dy);
                if (distance < (carwidth + other.carwidth) / 2) {
                    return true;
                }
            }
        }
        return false;
    }
};

struct Vehicle
{
    int lane, carlength, carwidth, x, y, speed;
    bool haschanged;
    QColor color;

    // 默认构造函数
    Vehicle(int l = 0, int cl = 0, int cw = 0, int x = 0, int y = 0, int s = 0, bool hc = false, QColor c = QColor(255, 255, 255),
            bool icl = false, bool igc = false, int tl = 0, float cp = 0.0f,
            int sx = 0, int sy = 0, int ex = 0, int ey = 0, bool itc = false, QColor oc = QColor(255, 255, 255), bool ibd = false)
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
    std::vector<std::pair<int, int>> laneChangeTrajectory; // 变道轨迹点集合

    // 距离警告相关成员
    bool isTooClose;      // 是否距离前车过近
    QColor originalColor; // 原始颜色

    // 抛锚状态
    bool isBrokenDown; // 车辆是否抛锚

    virtual void draw(QPainter &painter) const = 0;
    
    // 预测并绘制轨迹
    void predictAndDrawTrajectory(QPainter &painter, int laneHeight, int middleY, int roadWidth, const std::vector<Vehicle *> &allVehicles) const;

    // 前向运动函数
    void moveForward(int middleY)
    {
        x += (y < middleY) ? speed : -speed;
    }

    // 计算车辆之间的距离
    int calculateDistance(const Vehicle *other, int middleY) const;

    // 平滑变道函数
    bool smoothLaneChange(int laneHeight, const std::vector<Vehicle *> &allVehicles);
    virtual int getSafeDistance() const;
    
    // 修改变道曲线函数为虚函数，允许子类重写
    virtual float curveFunc(float t) const { return 3 * t * t - 2 * t * t * t; }
    
    // 检查前车距离并处理跟随转向
    void checkFrontVehicleDistance(std::vector<Vehicle *> &allVehicles, int safeDistance, int laneHeight, int stoppingSpeed);
    
    // 显示闪烁框（用于警告或变道提示）
    void showFlashingFrame(QPainter &painter, int roadWidth, bool isAttempting = false) const;
    
    // 处理危险情况（碰撞）
    void handleDangerousSituation();
};

struct Sedan : public Vehicle
{
    Sedan(int lane, int carlength, int carwidth, int x, int y, int speed);
    void draw(QPainter &painter) const override;
    int getSafeDistance() const; // 获取安全距离
};

struct SUV : public Vehicle
{
    SUV(int lane, int carlength, int carwidth, int x, int y, int speed);
    void draw(QPainter &painter) const override;
    int getSafeDistance() const; // 获取安全距离
};

struct Truck : public Vehicle
{
    Truck(int lane, int carlength, int carwidth, int x, int y, int speed);
    void draw(QPainter &painter) const override;
    int getSafeDistance() const; // 获取安全距离
};

struct Bridge
{
    double bridgeLength, bridgeWidth, widthScale;
    void calculateWindowSize(int &windowWidth, int &windowHeight, double &scale) const;
};

class QtWidgetsApplication1 : public QWidget
{
    Q_OBJECT

public:
    QtWidgetsApplication1(QWidget *parent = nullptr);
    ~QtWidgetsApplication1();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void updateSimulation();
    void on_freqPlus_clicked();
    void on_freqMinus_clicked();
    void on_distPlus_clicked();
    void on_distMinus_clicked();
    void on_speedPlus_clicked();
    void on_speedMinus_clicked();
    void on_clearLane0_clicked();
    void on_clearLane1_clicked();
    void on_clearLane2_clicked();
    void on_clearLane3_clicked();
    void on_clearLane4_clicked();
    void on_clearLane5_clicked();
    void on_exitButton_clicked();
    void on_normalButton_clicked();
    void on_rainButton_clicked();
    void on_snowButton_clicked();
    void on_dayButton_clicked();
    void on_nightButton_clicked();

private:
    Ui::QtWidgetsApplication1Class ui;

    // 模拟参数
    std::vector<Vehicle *> vehicles;
    QTimer *simulationTimer;
    double simulationTime;
    int vehicleGenerationFrequency;
    int safeDistance;
    int stoppingSpeed;
    WeatherMode currentWeather;
    TimeOfDay currentTime;
    double currentIlluminance;

    // UI布局参数
    int roadWidth;
    int controlBarWidth;
    int topBarHeight;
    int windowWidth;
    int windowHeight;
    double scale;

    // 桥梁参数
    Bridge bridge;

    // 随机数生成器
    std::mt19937 rng;
    std::normal_distribution<> normalwidth;
    std::normal_distribution<> normallength;
    std::uniform_int_distribution<int> int_dist;

    // 私有方法
    void initializeSimulation();
    void drawUI(QPainter &painter);
    void drawWeatherEffect(QPainter &painter, const QColor &backgroundColor);
    void generateNewVehicle();
    void updateVehicles();
    void removeOffScreenVehicles();
    QColor getBackgroundColor() const;
    void applyWeatherToSafety();
    double getEnvironmentConfig(TimeOfDay time, int weatherMode) const;
    void clearLane(int lane);
};
