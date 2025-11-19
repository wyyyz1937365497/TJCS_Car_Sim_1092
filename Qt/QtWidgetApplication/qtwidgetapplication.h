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

    // 距离警告相关成员
    bool isTooClose;      // 是否距离前车过近
    QColor originalColor; // 原始颜色

    // 抛锚状态
    bool isBrokenDown; // 车辆是否抛锚

    virtual void draw(QPainter &painter) const = 0;

    // 前向运动函数
    void moveForward(int middleY)
    {
        x += (y < middleY) ? speed : -speed;
    }
};

struct Sedan : public Vehicle
{
    Sedan(int lane, int carlength, int carwidth, int x, int y, int speed);
    void draw(QPainter &painter) const override;
};

struct SUV : public Vehicle
{
    SUV(int lane, int carlength, int carwidth, int x, int y, int speed);
    void draw(QPainter &painter) const override;
};

struct Truck : public Vehicle
{
    Truck(int lane, int carlength, int carwidth, int x, int y, int speed);
    void draw(QPainter &painter) const override;
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
    void generateNewVehicle();
    void updateVehicles();
    void removeOffScreenVehicles();
    QColor getBackgroundColor() const;
    void applyWeatherToSafety();
    double getEnvironmentConfig(TimeOfDay time, int weatherMode) const;
    void clearLane(int lane);
};