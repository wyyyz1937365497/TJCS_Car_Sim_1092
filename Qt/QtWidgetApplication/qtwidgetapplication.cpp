#include "qtwidgetapplication.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QTimer>
#include <QDateTime>
#include <QMessageBox>
#include <cmath>
#include <algorithm>
#include <random>

// 定义常量
#define SAFE_DISTANCE 150

// Sedan 实现
Sedan::Sedan(int lane, int carlength, int carwidth, int x, int y, int speed)
    : Vehicle(lane, carlength, carwidth, x, y, speed, false, QColor(0, 0, 255))
{
}

void Sedan::draw(QPainter &painter) const
{
    painter.setBrush(color);
    painter.setPen(Qt::black);
    painter.drawRect(x - carlength / 2, y - carwidth / 2, carlength, carwidth);

    if (isTooClose)
    {
        painter.setPen(QPen(Qt::red, 2));
        painter.drawRect(x - carlength / 2 - 2, y - carwidth / 2 - 2, carlength + 4, carwidth + 4);
    }
}

// SUV 实现
SUV::SUV(int lane, int carlength, int carwidth, int x, int y, int speed)
    : Vehicle(lane, carlength, carwidth, x, y, speed, false, QColor(0, 255, 0))
{
}

void SUV::draw(QPainter &painter) const
{
    painter.setBrush(color);
    painter.setPen(Qt::black);
    painter.drawRect(x - carlength / 2, y - carwidth / 2, carlength, carwidth);

    if (isTooClose)
    {
        painter.setPen(QPen(Qt::red, 2));
        painter.drawRect(x - carlength / 2 - 2, y - carwidth / 2 - 2, carlength + 4, carwidth + 4);
    }
}

// Truck 实现
Truck::Truck(int lane, int carlength, int carwidth, int x, int y, int speed)
    : Vehicle(lane, carlength, carwidth, x, y, speed, false, QColor(255, 255, 0))
{
}

void Truck::draw(QPainter &painter) const
{
    painter.setBrush(color);
    painter.setPen(Qt::black);
    painter.drawRect(x - carlength / 2, y - carwidth / 2, carlength, carwidth);

    if (isTooClose)
    {
        painter.setPen(QPen(Qt::red, 2));
        painter.drawRect(x - carlength / 2 - 2, y - carwidth / 2 - 2, carlength + 4, carwidth + 4);
    }
}

// Bridge 实现
void Bridge::calculateWindowSize(int &windowWidth, int &windowHeight, double &scale) const
{
    windowWidth = 1200;
    windowHeight = 800;
    scale = 10.0;
}

QtWidgetsApplication1::QtWidgetsApplication1(QWidget *parent)
    : QWidget(parent), simulationTimer(new QTimer(this)), simulationTime(0.0), vehicleGenerationFrequency(10), safeDistance(SAFE_DISTANCE), stoppingSpeed(15), currentWeather(NOTHING), currentTime(TimeOfDay::Day), currentIlluminance(0.0), controlBarWidth(60), topBarHeight(80), rng(QDateTime::currentMSecsSinceEpoch()), normalwidth(3, 0.1), normallength(6, 0.1), int_dist(20, 120)
{
    ui.setupUi(this);

    // 设置窗口大小
    resize(1200, 800);

    // 初始化桥梁参数
    bridge.bridgeLength = 100;
    bridge.bridgeWidth = 50;
    bridge.widthScale = 1;

    // 计算窗口尺寸
    bridge.calculateWindowSize(windowWidth, windowHeight, scale);
    roadWidth = windowWidth - controlBarWidth;

    // 连接定时器信号与槽
    connect(simulationTimer, &QTimer::timeout, this, &QtWidgetsApplication1::updateSimulation);
    simulationTimer->start(40); // 40ms更新一次，约25FPS

    // 应用初始天气设置
    applyWeatherToSafety();
}

QtWidgetsApplication1::~QtWidgetsApplication1()
{
    // 清理车辆内存
    for (auto v : vehicles)
    {
        delete v;
    }
    vehicles.clear();
}

void QtWidgetsApplication1::initializeSimulation()
{
    // 初始化模拟参数
    simulationTime = 0.0;
    vehicleGenerationFrequency = 10;
    safeDistance = SAFE_DISTANCE;
    stoppingSpeed = 15;
    currentWeather = NOTHING;
    currentTime = TimeOfDay::Day;
}

void QtWidgetsApplication1::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 绘制道路背景（根据时间变化）
    painter.fillRect(0, topBarHeight, roadWidth, windowHeight - topBarHeight, getBackgroundColor());

    // 绘制车道线
    painter.setPen(QPen(Qt::white, 1, Qt::DashLine));
    int laneCount = 6;
    int laneHeight = (windowHeight - topBarHeight) / laneCount;
    for (int i = 0; i < laneCount - 1; ++i)
    {
        int y = topBarHeight + (i + 1) * laneHeight;
        painter.drawLine(0, y, roadWidth, y);
    }

    // 绘制车辆
    for (const auto &v : vehicles)
    {
        v->draw(painter);
    }

    // 重置画笔颜色，确保菜单绘制不受车辆颜色影响
    painter.setPen(Qt::NoPen);

    // 绘制UI（固定颜色，不随时间变化）
    drawUI(painter);
}

void QtWidgetsApplication1::mousePressEvent(QMouseEvent *event)
{
    // 处理鼠标点击事件
    QWidget::mousePressEvent(event);
}

void QtWidgetsApplication1::updateSimulation()
{
    // 更新模拟时间
    simulationTime += 0.04; // 每40ms增加0.04秒

    // 生成新车辆
    generateNewVehicle();

    // 更新车辆状态
    updateVehicles();

    // 移除屏幕外的车辆
    removeOffScreenVehicles();

    // 触发重绘
    update();
}

void QtWidgetsApplication1::generateNewVehicle()
{
    // 根据生成频率控制变量决定是否生成新车
    if (rng() % vehicleGenerationFrequency == 0)
    {
        int lane = rng() % 6;
        int carwidth = (int)(normalwidth(rng) * scale * bridge.widthScale);
        int carlength = (int)(normallength(rng) * scale);

        int laneCount = 6;
        int laneHeight = (windowHeight - topBarHeight) / laneCount;
        int newY = topBarHeight + laneHeight * lane + (int)(0.5 * laneHeight);

        // 起始 X：向右行驶的车辆从左侧进入，向左行驶的车辆从右侧进入
        int newX = (lane < 3) ? (-carlength / 2 - 5) : (roadWidth + carlength / 2 + 5);

        int vehicleType = rng() % 3;
        Vehicle *newVehicle = nullptr;
        int speed = int_dist(rng);

        if (vehicleType == 0)
        {
            newVehicle = new Sedan(lane, carwidth, carlength, newX, newY, speed);
        }
        else if (vehicleType == 1)
        {
            newVehicle = new SUV(lane, carwidth, carlength, newX, newY, speed);
        }
        else
        {
            newVehicle = new Truck(lane, carwidth, carlength, newX, newY, speed);
        }

        if (newVehicle)
        {
            vehicles.push_back(newVehicle);
        }
    }
}

void QtWidgetsApplication1::updateVehicles()
{
    int middleY = (windowHeight + topBarHeight) / 2;
    int laneCount = 6;
    int laneHeight = (windowHeight - topBarHeight) / laneCount;

    for (auto &v : vehicles)
    {
        // 简化的车辆移动逻辑
        v->moveForward(middleY);
    }
}

void QtWidgetsApplication1::removeOffScreenVehicles()
{
    // 移除离开屏幕的车辆
    for (auto it = vehicles.begin(); it != vehicles.end();)
    {
        Vehicle *v = *it;
        if ((v->lane < 3 && v->x - v->carlength / 2 > roadWidth) ||
            (v->lane >= 3 && v->x + v->carlength / 2 < 0))
        {
            delete v;
            it = vehicles.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

QColor QtWidgetsApplication1::getBackgroundColor() const
{
    // 根据当前时间设置背景色
    if (currentTime == TimeOfDay::Day)
    {
        return QColor(100, 100, 100); // 白天为灰色
    }
    else
    {
        return QColor(0, 0, 0); // 夜晚为黑色
    }
}

void QtWidgetsApplication1::drawUI(QPainter &painter)
{
    // 重置画笔和画刷状态，确保菜单颜色不受之前绘制的影响
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush());

    // 绘制顶部控制栏（固定深灰色）
    painter.fillRect(0, 0, windowWidth, topBarHeight, QColor(40, 40, 40));
    painter.setPen(QPen(QColor(80, 80, 80)));
    painter.drawRect(0, 0, windowWidth, topBarHeight);

    // 绘制右侧控制栏（固定深灰色）
    painter.fillRect(roadWidth, topBarHeight, controlBarWidth, windowHeight - topBarHeight, QColor(50, 50, 50));
    painter.setPen(QPen(QColor(100, 100, 100)));
    painter.drawRect(roadWidth, topBarHeight, controlBarWidth, windowHeight - topBarHeight);

    // 显示桥梁信息
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);
    QString bridgeInfo = QString("桥长: %1m  桥宽: %2m  桥宽放大率: %3")
                             .arg(bridge.bridgeLength)
                             .arg(bridge.bridgeWidth)
                             .arg(bridge.widthScale);
    painter.drawText(10, 20, bridgeInfo);

    // 显示时间
    QString timeInfo = QString("时间: %1s").arg((int)simulationTime);
    painter.drawText(roadWidth - 150, topBarHeight + 25, timeInfo);

    // 显示天气状态
    QString weatherText = "正常";
    switch (currentWeather)
    {
    case RAIN:
        weatherText = "下雨";
        break;
    case SNOW:
        weatherText = "下雪";
        break;
    }

    QString weatherInfo = QString("当前天气: %1").arg(weatherText);
    painter.drawText(roadWidth / 2 - 50, 35, weatherInfo);

    // 显示参数信息
    QString paramInfo = QString("生成频率:%1 探测距离:%2 减速度:%3 照度: %4 lux")
                            .arg(vehicleGenerationFrequency)
                            .arg(safeDistance)
                            .arg(stoppingSpeed)
                            .arg(currentIlluminance, 0, 'f', 2);
    painter.drawText(10, topBarHeight - 10, paramInfo);

    // 绘制天气控制按钮状态
    QColor activeColor(0, 120, 215);
    QColor inactiveColor(70, 70, 70);

    QColor normalBtnColor = (currentWeather == NOTHING) ? activeColor : inactiveColor;
    QColor rainBtnColor = (currentWeather == RAIN) ? activeColor : inactiveColor;
    QColor snowBtnColor = (currentWeather == SNOW) ? activeColor : inactiveColor;

    QColor dayBtnColor = (currentTime == TimeOfDay::Day) ? activeColor : inactiveColor;
    QColor nightBtnColor = (currentTime == TimeOfDay::Night) ? activeColor : inactiveColor;

    // 绘制按钮状态指示
    painter.fillRect(ui.normalButton->x(), ui.normalButton->y(),
                     ui.normalButton->width(), ui.normalButton->height(), normalBtnColor);

    painter.fillRect(ui.rainButton->x(), ui.rainButton->y(),
                     ui.rainButton->width(), ui.rainButton->height(), rainBtnColor);

    painter.fillRect(ui.snowButton->x(), ui.snowButton->y(),
                     ui.snowButton->width(), ui.snowButton->height(), snowBtnColor);

    painter.fillRect(ui.dayButton->x(), ui.dayButton->y(),
                     ui.dayButton->width(), ui.dayButton->height(), dayBtnColor);

    painter.fillRect(ui.nightButton->x(), ui.nightButton->y(),
                     ui.nightButton->width(), ui.nightButton->height(), nightBtnColor);
}

void QtWidgetsApplication1::applyWeatherToSafety()
{
    try
    {
        double illuminance_lux = getEnvironmentConfig(currentTime, currentWeather);
        currentIlluminance = illuminance_lux;

        // 根据照度调整 safeDistance
        double scaledMeters;
        if (illuminance_lux >= 100000.0)
        {
            scaledMeters = 1200.0; // 白天晴天
        }
        else if (illuminance_lux >= 60000.0)
        {
            scaledMeters = 800.0; // 白天雪天
        }
        else if (illuminance_lux >= 20000.0)
        {
            scaledMeters = 400.0; // 白天雨天
        }
        else if (illuminance_lux >= 10000.0)
        {
            scaledMeters = 200.0; // 夜晚晴天
        }
        else if (illuminance_lux >= 6000.0)
        {
            scaledMeters = 100.0; // 夜晚雨天
        }
        else
        {
            scaledMeters = 50.0; // 夜晚雪天
        }

        safeDistance = (int)scaledMeters;

        // 根据天气调整 stoppingSpeed
        const int baseStopping = 15;
        if (currentWeather == NOTHING)
        {
            stoppingSpeed = baseStopping;
        }
        else if (currentWeather == RAIN)
        {
            stoppingSpeed = std::max(1, (int)(baseStopping * 0.8));
        }
        else
        { // SNOW
            stoppingSpeed = std::max(1, (int)(baseStopping * 0.6));
        }
    }
    catch (...)
    {
        // 若获取环境配置失败，不改变现有值
    }
}

double QtWidgetsApplication1::getEnvironmentConfig(TimeOfDay time, int weatherMode) const
{
    // 根据时间和天气模式返回照度值
    if (time == TimeOfDay::Day)
    {
        switch (weatherMode)
        {
        case NOTHING:
            return 100000.0; // 白天晴天
        case RAIN:
            return 20000.0; // 白天雨天
        case SNOW:
            return 60000.0; // 白天雪天
        }
    }
    else
    {
        switch (weatherMode)
        {
        case NOTHING:
            return 10000.0; // 夜晚晴天
        case RAIN:
            return 6000.0; // 夜晚雨天
        case SNOW:
            return 2000.0; // 夜晚雪天
        }
    }
    return 10000.0;
}

void QtWidgetsApplication1::clearLane(int lane)
{
    // 清除指定车道上的所有车辆
    for (auto it = vehicles.begin(); it != vehicles.end();)
    {
        Vehicle *v = *it;
        if (v->lane == lane)
        {
            delete v;
            it = vehicles.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

// 按钮槽函数实现
void QtWidgetsApplication1::on_freqPlus_clicked()
{
    vehicleGenerationFrequency = std::min(vehicleGenerationFrequency + 1, 100);
    update();
}

void QtWidgetsApplication1::on_freqMinus_clicked()
{
    vehicleGenerationFrequency = std::max(vehicleGenerationFrequency - 1, 1);
    update();
}

void QtWidgetsApplication1::on_distPlus_clicked()
{
    safeDistance = std::min(safeDistance + 50, 2000);
    update();
}

void QtWidgetsApplication1::on_distMinus_clicked()
{
    safeDistance = std::max(safeDistance - 50, 100);
    update();
}

void QtWidgetsApplication1::on_speedPlus_clicked()
{
    stoppingSpeed = std::min(stoppingSpeed + 1, 50);
    update();
}

void QtWidgetsApplication1::on_speedMinus_clicked()
{
    stoppingSpeed = std::max(stoppingSpeed - 1, 1);
    update();
}

void QtWidgetsApplication1::on_clearLane0_clicked() { clearLane(0); }
void QtWidgetsApplication1::on_clearLane1_clicked() { clearLane(1); }
void QtWidgetsApplication1::on_clearLane2_clicked() { clearLane(2); }
void QtWidgetsApplication1::on_clearLane3_clicked() { clearLane(3); }
void QtWidgetsApplication1::on_clearLane4_clicked() { clearLane(4); }
void QtWidgetsApplication1::on_clearLane5_clicked() { clearLane(5); }

void QtWidgetsApplication1::on_exitButton_clicked()
{
    close();
}

void QtWidgetsApplication1::on_normalButton_clicked()
{
    currentWeather = NOTHING;
    applyWeatherToSafety();
    update();
}

void QtWidgetsApplication1::on_rainButton_clicked()
{
    currentWeather = RAIN;
    applyWeatherToSafety();
    update();
}

void QtWidgetsApplication1::on_snowButton_clicked()
{
    currentWeather = SNOW;
    applyWeatherToSafety();
    update();
}

void QtWidgetsApplication1::on_dayButton_clicked()
{
    currentTime = TimeOfDay::Day;
    applyWeatherToSafety();
    update();
}

void QtWidgetsApplication1::on_nightButton_clicked()
{
    currentTime = TimeOfDay::Night;
    applyWeatherToSafety();
    update();
}