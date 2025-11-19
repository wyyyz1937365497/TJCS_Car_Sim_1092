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

// 随机数生成器
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> color_dis(0, 255);

// 生成真实车辆颜色的函数
QColor generateVehicleColor() {
    // 定义一些常见的车辆颜色（排除黑色）
    static const QColor commonColors[] = {
        QColor(255, 255, 255), // 白色
        QColor(255, 0, 0),     // 红色
        QColor(0, 0, 255),     // 蓝色
        QColor(255, 255, 0),   // 黄色
        QColor(0, 255, 0),     // 绿色
        QColor(255, 165, 0),   // 橙色
        QColor(128, 0, 128),   // 紫色
        QColor(165, 42, 42),   // 棕色
        QColor(192, 192, 192), // 银色
        QColor(128, 128, 128), // 灰色
        QColor(255, 192, 203), // 粉色
        QColor(75, 0, 130),    // 靛蓝色
        QColor(240, 230, 140), // 像牙黄
        QColor(210, 180, 140), // 卡其色
        QColor(255, 182, 193), // 浅粉红
        QColor(176, 224, 230), // 粉蓝色
        QColor(144, 238, 144), // 浅绿色
        QColor(221, 160, 221), // 梅花色
        QColor(255, 105, 180)  // 热粉红
    };

    std::uniform_int_distribution<> index_dis(0, sizeof(commonColors) / sizeof(QColor) - 1);
    return commonColors[index_dis(gen)];
}

int Vehicle::getSafeDistance() const
{
    return SAFE_DISTANCE;
}

// Sedan 实现
Sedan::Sedan(int lane, int carlength, int carwidth, int x, int y, int speed)
    : Vehicle(lane, carlength, carwidth, x, y, speed, false, generateVehicleColor())
{
}

int Sedan::getSafeDistance() const
{
    return SAFE_DISTANCE * 0.8; // 比标准安全距离短20%
}

void Sedan::draw(QPainter &painter) const
{
    int left = x - carlength / 2;
    int right = x + carlength / 2;
    int top = y - carwidth / 2;
    int bottom = y + carwidth / 2;

    bool movingRight = (lane < 3);

    if (isBrokenDown)
    {
        // 绘制抛锚车辆
        painter.setBrush(QColor(100, 100, 100));
        painter.setPen(QPen(QColor(50, 50, 50), 2));
        painter.drawRoundedRect(left, top, carlength, carwidth, 8, 8);

        // 绘制X形标记表示抛锚
        painter.setPen(QPen(Qt::red, 3));
        painter.drawLine(x - carlength / 4, y - carwidth / 4, x + carlength / 4, y + carwidth / 4);
        painter.drawLine(x - carlength / 4, y + carwidth / 4, x + carlength / 4, y - carwidth / 4);
    }
    else
    {
        // 绘制正常的小轿车
        painter.setBrush(color);
        painter.setPen(QPen(QColor(30, 30, 30), 2));

        // 绘制车身主体（使用多边形模拟真实三厢结构）
        QPolygon body;
        if (movingRight) {
            // 向右行驶时：车头在右
            body << QPoint(left, bottom - carwidth / 8)                   // 后保险杠底部
                 << QPoint(left + carlength / 8, top + carwidth / 3)      // 后轮拱上方
                 << QPoint(left + carlength / 4, top + carwidth / 8)      // 车顶后部
                 << QPoint(left + carlength * 3 / 4, top + carwidth / 8)  // 车顶前部
                 << QPoint(right - carlength / 12, top + carwidth / 4)    // 发动机盖
                 << QPoint(right - carlength / 24, top + carwidth / 3)    // 车头顶部
                 << QPoint(right, bottom - carwidth / 4)                  // 车头底部
                 << QPoint(left, bottom - carwidth / 8);                  // 闭合
        } else {
            // 向左行驶时：车头在左
            body << QPoint(right, bottom - carwidth / 8)                  // 后保险杠底部
                 << QPoint(right - carlength / 8, top + carwidth / 3)     // 后轮拱上方
                 << QPoint(right - carlength / 4, top + carwidth / 8)     // 车顶后部
                 << QPoint(right - carlength * 3 / 4, top + carwidth / 8) // 车顶前部
                 << QPoint(left + carlength / 12, top + carwidth / 4)     // 发动机盖
                 << QPoint(left + carlength / 24, top + carwidth / 3)     // 车头顶部
                 << QPoint(left, bottom - carwidth / 4)                   // 车头底部
                 << QPoint(right, bottom - carwidth / 8);                 // 闭合
        }
        painter.drawPolygon(body);

        // 绘制车窗
        painter.setBrush(QColor(173, 216, 230)); // 浅蓝色
        painter.setPen(QPen(QColor(80, 80, 80), 1));
        
        QPolygon window;
        if (movingRight) {
            window << QPoint(left + carlength * 3 / 4, top + carwidth / 6)
                   << QPoint(right - carlength / 12, top + carwidth / 4)
                   << QPoint(right - carlength / 24, top + carwidth / 3)
                   << QPoint(left + carlength / 3, top + carwidth / 12)
                   << QPoint(left + carlength * 3 / 4, top + carwidth / 12)
                   << QPoint(left + carlength / 4, top + carwidth / 6)
                   << QPoint(left + carlength / 3, top + carwidth / 12)
                   << QPoint(left + carlength / 4, top + carwidth / 6);
        } else {
            window << QPoint(right - carlength * 3 / 4, top + carwidth / 6)
                   << QPoint(left + carlength / 12, top + carwidth / 4)
                   << QPoint(left + carlength / 24, top + carwidth / 3)
                   << QPoint(right - carlength / 3, top + carwidth / 12)
                   << QPoint(right - carlength * 3 / 4, top + carwidth / 12)
                   << QPoint(right - carlength / 4, top + carwidth / 6)
                   << QPoint(right - carlength / 3, top + carwidth / 12)
                   << QPoint(right - carlength / 4, top + carwidth / 6);
        }
        painter.drawPolygon(window);

        // 绘制车门分隔线
        painter.setPen(QPen(QColor(100, 100, 100), 1));
        if (movingRight) {
            painter.drawLine(left + carlength / 3, top + carwidth / 8, left + carlength / 3, bottom - carwidth / 6);
            painter.drawLine(left + carlength * 2 / 3, top + carwidth / 8, left + carlength * 2 / 3, bottom - carwidth / 6);
        } else {
            painter.drawLine(right - carlength / 3, top + carwidth / 8, right - carlength / 3, bottom - carwidth / 6);
            painter.drawLine(right - carlength * 2 / 3, top + carwidth / 8, right - carlength * 2 / 3, bottom - carwidth / 6);
        }

        // 绘制车灯
        painter.setBrush(movingRight ? QColor(255, 255, 200) : Qt::red); // 前灯
        if (movingRight) {
            painter.drawRect(right - carlength / 12, top + carwidth / 4, carlength / 20, carwidth / 6);
        } else {
            painter.drawRect(left, top + carwidth / 4, carlength / 20, carwidth / 6);
        }

        painter.setBrush(movingRight ? Qt::red : QColor(255, 255, 200)); // 尾灯
        if (movingRight) {
            painter.drawRect(left, top + carwidth / 4, carlength / 20, carwidth / 6);
        } else {
            painter.drawRect(right - carlength / 20, top + carwidth / 4, carlength / 20, carwidth / 6);
        }

        // 绘制车轮
        painter.setBrush(Qt::black);
        painter.setPen(QPen(QColor(50, 50, 50), 1));
        int wheelRadius = std::max(5, carwidth / 5);
        int wheelY = bottom - wheelRadius / 3;
        int frontWheelX, rearWheelX;

        if (movingRight) {
            frontWheelX = right - carlength / 5;
            rearWheelX = left + carlength / 5;
        } else {
            frontWheelX = left + carlength / 5;
            rearWheelX = right - carlength / 5;
        }

        painter.drawEllipse(frontWheelX - wheelRadius, wheelY - wheelRadius, wheelRadius * 2, wheelRadius * 2);
        painter.drawEllipse(rearWheelX - wheelRadius, wheelY - wheelRadius, wheelRadius * 2, wheelRadius * 2);
    }

    // 绘制速度信息
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10));
    painter.drawText(x - 10, top - 10, QString::number(speed));
    
    // 恢复画笔和画刷到默认状态，确保不影响后续绘制（如警告框）
    painter.setBrush(Qt::NoBrush);
}

// SUV 实现
SUV::SUV(int lane, int carlength, int carwidth, int x, int y, int speed)
    : Vehicle(lane, carlength, carwidth, x, y, speed, false, generateVehicleColor())
{
}

int SUV::getSafeDistance() const
{
    return SAFE_DISTANCE;
}

void SUV::draw(QPainter &painter) const
{
    int left = x - carlength / 2;
    int right = x + carlength / 2;
    int top = y - carwidth / 2;
    int bottom = y + carwidth / 2;

    bool movingRight = (lane < 3);

    if (isBrokenDown)
    {
        // 绘制抛锚车辆
        painter.setBrush(QColor(100, 100, 100));
        painter.setPen(QPen(QColor(50, 50, 50), 2));
        painter.drawRoundedRect(left, top, carlength, carwidth, 8, 8);

        // 绘制X形标记表示抛锚
        painter.setPen(QPen(Qt::red, 3));
        painter.drawLine(x - carlength / 4, y - carwidth / 4, x + carlength / 4, y + carwidth / 4);
        painter.drawLine(x - carlength / 4, y + carwidth / 4, x + carlength / 4, y - carwidth / 4);
    }
    else
    {
        // 绘制正常的SUV
        painter.setBrush(color);
        painter.setPen(QPen(QColor(30, 30, 30), 2));

        // 绘制SUV车身主体（更方正，车顶线条平直）
        QPolygon body;
        if (movingRight) {
            // 向右行驶时：车头在右
            body << QPoint(left, bottom - carwidth / 6)                   // 后保险杠底部
                 << QPoint(left + carlength / 8, top + carwidth / 8)      // 后轮拱上方
                 << QPoint(left + carlength / 4, top)                     // 车顶后部
                 << QPoint(left + carlength * 4 / 5, top)                 // 车顶前部
                 << QPoint(right - carlength / 12, top + carwidth / 6)    // 发动机盖
                 << QPoint(right, top + carwidth / 3)                     // 车头
                 << QPoint(right, bottom - carwidth / 8)                  // 车头底部
                 << QPoint(left, bottom - carwidth / 6);                  // 闭合
        } else {
            // 向左行驶时：车头在左
            body << QPoint(right, bottom - carwidth / 6)                  // 后保险杠底部
                 << QPoint(right - carlength / 8, top + carwidth / 8)     // 后轮拱上方
                 << QPoint(right - carlength / 4, top)                    // 车顶后部
                 << QPoint(right - carlength * 4 / 5, top)                // 车顶前部
                 << QPoint(left + carlength / 12, top + carwidth / 6)     // 发动机盖
                 << QPoint(left, top + carwidth / 3)                      // 车头
                 << QPoint(left, bottom - carwidth / 8)                   // 车头底部
                 << QPoint(right, bottom - carwidth / 6);                 // 闭合
        }
        painter.drawPolygon(body);

        // 绘制SUV车窗（更大面积的前挡风玻璃）
        painter.setBrush(QColor(135, 206, 250)); // 天蓝色
        painter.setPen(QPen(QColor(80, 80, 80), 1));
        
        QPolygon window;
        if (movingRight) {
            window << QPoint(left + carlength * 4 / 5, top + carwidth / 12)
                   << QPoint(right - carlength / 12, top + carwidth / 4)
                   << QPoint(right - carlength / 20, top + carwidth / 3)
                   << QPoint(left + carlength / 3, top + carwidth / 12)
                   << QPoint(left + carlength * 4 / 5, top + carwidth / 12)
                   << QPoint(left + carlength / 3, top + carwidth / 12);
        } else {
            window << QPoint(right - carlength * 4 / 5, top + carwidth / 12)
                   << QPoint(left + carlength / 12, top + carwidth / 4)
                   << QPoint(left + carlength / 20, top + carwidth / 3)
                   << QPoint(right - carlength / 3, top + carwidth / 12)
                   << QPoint(right - carlength * 4 / 5, top + carwidth / 12)
                   << QPoint(right - carlength / 3, top + carwidth / 12);
        }
        painter.drawPolygon(window);

        // 绘制SUV车门分隔线（通常有4个门）
        painter.setPen(QPen(QColor(100, 100, 100), 1));
        if (movingRight) {
            painter.drawLine(left + carlength / 3, top + carwidth / 8, left + carlength / 3, bottom - carwidth / 6);
            painter.drawLine(left + carlength * 2 / 3, top + carwidth / 8, left + carlength * 2 / 3, bottom - carwidth / 6);
        } else {
            painter.drawLine(right - carlength / 3, top + carwidth / 8, right - carlength / 3, bottom - carwidth / 6);
            painter.drawLine(right - carlength * 2 / 3, top + carwidth / 8, right - carlength * 2 / 3, bottom - carwidth / 6);
        }

        // 绘制橙色装饰条
        painter.setBrush(QColor(255, 165, 0)); // 橙色
        painter.setPen(QPen(QColor(255, 165, 0), 1));
        if (movingRight) {
            painter.drawRect(left, bottom - carwidth / 4, carlength, carwidth / 20);
        } else {
            painter.drawRect(left, bottom - carwidth / 4, carlength, carwidth / 20);
        }

        // 绘制车灯
        painter.setBrush(movingRight ? QColor(255, 255, 200) : Qt::red); // 前灯
        if (movingRight) {
            painter.drawRect(right - carlength / 12, top + carwidth / 5, carlength / 20, carwidth / 6);
        } else {
            painter.drawRect(left, top + carwidth / 5, carlength / 20, carwidth / 6);
        }

        painter.setBrush(movingRight ? Qt::red : QColor(255, 255, 200)); // 尾灯
        if (movingRight) {
            painter.drawRect(left, top + carwidth / 5, carlength / 20, carwidth / 6);
        } else {
            painter.drawRect(right - carlength / 20, top + carwidth / 5, carlength / 20, carwidth / 6);
        }

        // 绘制SUV车轮（通常更大）
        painter.setBrush(Qt::black);
        painter.setPen(QPen(QColor(50, 50, 50), 1));
        int wheelRadius = std::max(5, carwidth / 4); // SUV车轮更大
        int wheelY = bottom - wheelRadius / 3; // 离地间隙更大
        int frontWheelX, rearWheelX;

        if (movingRight) {
            frontWheelX = right - carlength / 6; // 前轮位置更靠前
            rearWheelX = left + carlength / 6;
        } else {
            frontWheelX = left + carlength / 6; // 前轮位置更靠前
            rearWheelX = right - carlength / 6;
        }

        painter.drawEllipse(frontWheelX - wheelRadius, wheelY - wheelRadius, wheelRadius * 2, wheelRadius * 2);
        painter.drawEllipse(rearWheelX - wheelRadius, wheelY - wheelRadius, wheelRadius * 2, wheelRadius * 2);
    }

    // 绘制速度信息
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10));
    painter.drawText(x - 10, top - 10, QString::number(speed));

    // 恢复画笔和画刷到默认状态，确保不影响后续绘制（如警告框）
    painter.setBrush(Qt::NoBrush);
    painter.setPen(Qt::NoPen);
}

// Truck 实现
Truck::Truck(int lane, int carlength, int carwidth, int x, int y, int speed)
    : Vehicle(lane, carlength, carwidth, x, y, speed, false, generateVehicleColor())
{
}

int Truck::getSafeDistance() const
{
    return SAFE_DISTANCE * 1.5; // 比标准安全距离长50%
}

void Truck::draw(QPainter &painter) const
{
    int left = x - carlength / 2;
    int right = x + carlength / 2;
    int top = y - carwidth / 2;
    int bottom = y + carwidth / 2;

    bool movingRight = (lane < 3);

    if (isBrokenDown)
    {
        // 绘制抛锚车辆
        painter.setBrush(QColor(100, 100, 100));
        painter.setPen(QPen(QColor(50, 50, 50), 2));
        painter.drawRoundedRect(left, top, carlength, carwidth, 8, 8);

        // 绘制X形标记表示抛锚
        painter.setPen(QPen(Qt::red, 3));
        painter.drawLine(x - carlength / 4, y - carwidth / 4, x + carlength / 4, y + carwidth / 4);
        painter.drawLine(x - carlength / 4, y + carwidth / 4, x + carlength / 4, y - carwidth / 4);
    }
    else
    {
        // 绘制正常的货车（驾驶舱 + 货厢）
        int cabLength = std::max(20, carlength / 4);  // 驾驶舱长度
        int cabLeft, cabRight, cargoLeft, cargoRight;

        if (movingRight) {
            cabLeft = right - cabLength;
            cabRight = right;
            cargoLeft = left;
            cargoRight = cabLeft;
        } else {
            cabLeft = left;
            cabRight = left + cabLength;
            cargoLeft = cabRight;
            cargoRight = right;
        }

        painter.setPen(QPen(QColor(30, 30, 30), 2));

        // 1. 绘制货厢（全封闭，没有窗户）
        painter.setBrush(color);
        painter.drawRect(cargoLeft, top, cargoRight - cargoLeft, carwidth);

        // 绘制货厢竖筋
        painter.setPen(QPen(QColor(100, 100, 100), 1));
        int ribCount = std::max(3, abs(cargoRight - cargoLeft) / 25);
        for (int i = 1; i < ribCount; ++i) {
            int ribX = cargoLeft + i * (cargoRight - cargoLeft) / ribCount;
            painter.drawLine(ribX, top, ribX, bottom);
        }

        // 2. 绘制驾驶舱
        painter.setBrush(QColor(220, 220, 220));
        painter.setPen(QPen(QColor(30, 30, 30), 2));
        painter.drawRoundedRect(cabLeft, top, cabRight - cabLeft, carwidth, 6, 6);

        // 绘制驾驶舱车窗
        int cabWindowTop = top + carwidth / 3;
        int cabWindowBot = top + carwidth * 2 / 3;
        int cabWindowLeft = cabLeft + cabLength / 8;
        int cabWindowRight = cabRight - cabLength / 8;

        painter.setBrush(QColor(135, 206, 250)); // 天蓝色
        painter.setPen(QPen(QColor(80, 80, 80), 1));
        painter.drawRect(cabWindowLeft, cabWindowTop, cabWindowRight - cabWindowLeft, cabWindowBot - cabWindowTop);

        // 绘制驾驶舱车门分隔
        painter.setPen(QPen(QColor(100, 100, 100), 1));
        int cabDoor = (cabLeft + cabRight) / 2;
        painter.drawLine(cabDoor, cabWindowTop, cabDoor, cabWindowBot);
        painter.drawLine(cabDoor, cabWindowBot, cabDoor, bottom - carwidth / 5);

        // 绘制车灯
        painter.setBrush(QColor(255, 255, 200)); // 前灯
        int lightSize = std::max(3, carwidth / 6);
        int lightY = cabWindowTop;
        if (movingRight) {
            painter.drawRect(cabRight - 4, lightY, 4, lightSize);
        } else {
            painter.drawRect(cabLeft, lightY, 4, lightSize);
        }

        painter.setBrush(Qt::red); // 尾灯
        if (movingRight) {
            painter.drawRect(cargoLeft, lightY, 4, lightSize);
        } else {
            painter.drawRect(cargoRight - 4, lightY, 4, lightSize);
        }

        // 绘制车轮（三轴）
        painter.setBrush(Qt::black);
        painter.setPen(QPen(QColor(50, 50, 50), 1));
        int wheelRadius = std::max(4, carwidth / 4);
        int wheelY = bottom - wheelRadius / 2;

        // 驾驶舱轮
        int cabWheelX = (cabLeft + cabRight) / 2;
        painter.drawEllipse(cabWheelX - wheelRadius, wheelY - wheelRadius, wheelRadius * 2, wheelRadius * 2);

        // 货厢双轮
        int cargo1X = cargoLeft + abs(cargoRight - cargoLeft) * 2 / 3;
        int cargo2X = cargoLeft + abs(cargoRight - cargoLeft) * 5 / 6;
        painter.drawEllipse(cargo1X - wheelRadius, wheelY - wheelRadius, wheelRadius * 2, wheelRadius * 2);
        painter.drawEllipse(cargo2X - wheelRadius, wheelY - wheelRadius, wheelRadius * 2, wheelRadius * 2);
    }

    // 绘制速度信息
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10));
    painter.drawText(x - 10, top - 10, QString::number(speed));
}

// Vehicle 实现
int Vehicle::calculateDistance(const Vehicle *other, int middleY) const
{
    // 计算两辆车之间的距离
    if ((this->y < middleY && other->y < middleY) ||
        (this->y >= middleY && other->y >= middleY))
    {
        // 同向行驶
        return abs(this->x - other->x);
    }
    return INT_MAX; // 不同方向行驶，认为距离无限大
}

bool Vehicle::smoothLaneChange(int laneHeight, const std::vector<Vehicle *> &allVehicles)
{
    // 如果车辆已抛锚，不能变道
    if (isBrokenDown)
    {
        isGoing2change = false;
        return false;
    }

    if (isChangingLane)
    {
        // 更新变道进度，基于车辆类型调整变道速度
        // 小轿车变道较快，卡车较慢
        float changeSpeed = 0.02f;
        if (dynamic_cast<const Sedan *>(this))
        {
            changeSpeed = 0.025f; // 小轿车变道稍快
        }
        else if (dynamic_cast<const Truck *>(this))
        {
            changeSpeed = 0.015f; // 卡车变道稍慢
        }

        changeProgress += changeSpeed;

        if (changeProgress >= 1.0f)
        {
            // 变道完成
            changeProgress = 1.0f;
            isChangingLane = false;
            isGoing2change = false;
            lane = targetLane;

            // 清空轨迹点
            laneChangeTrajectory.clear();
            return true;
        }

        // 使用预计算的轨迹点更新车辆位置
        int trajectoryIndex = (int)(changeProgress * (laneChangeTrajectory.size() - 1));
        if (trajectoryIndex >= laneChangeTrajectory.size())
        {
            trajectoryIndex = laneChangeTrajectory.size() - 1;
        }

        x = laneChangeTrajectory[trajectoryIndex].first;
        y = laneChangeTrajectory[trajectoryIndex].second;

        return false;
    }

    // 确定目标车道
    int tempTargetLane = 0;
    if (lane == 0 || lane == 3)
    {
        tempTargetLane = lane + 1;
    }
    else if (lane == 2 || lane == 5)
    {
        tempTargetLane = lane - 1;
    }
    else if (lane == 1 || lane == 4)
    {
        tempTargetLane = lane + (rand() % 2 ? 1 : -1);
    }

    // 预先计算整个变道轨迹点
    laneChangeTrajectory.clear();
    const int predictionSteps = 50; // 增加轨迹点数量以获得更平滑的移动

    // 水平方向保持匀速运动
    int currentX = x;
    int targetX = x + (y < laneHeight * 3 ? 1 : -1) * abs(endX - startX); // 水平方向移动距离
    int deltaX = targetX - currentX;

    // 垂直方向目标位置 (考虑顶部面板高度偏移)
    int currentY = y;
    int targetY = 80 + laneHeight * tempTargetLane + (int)(0.5 * laneHeight); // 添加80像素的顶部面板偏移
    int deltaY = targetY - currentY;

    // 水平方向速度保持不变
    int horizontalSpeed = (y < laneHeight * 3) ? speed : -speed;
    int totalHorizontalDistance = horizontalSpeed * 30; // 控制水平移动距离

    for (int i = 0; i <= predictionSteps; ++i)
    {
        // 计算进度
        float t = i * (1.0f / predictionSteps);

        // 水平方向匀速运动
        int newX = currentX + (int)(totalHorizontalDistance * t);

        // 垂直方向使用贝塞尔曲线
        float verticalProgress = curveFunc(t);
        int newY = currentY + (int)(deltaY * verticalProgress);

        // 添加到轨迹
        laneChangeTrajectory.push_back(std::make_pair(newX, newY));
    }

    // 检查目标车道是否有足够空间
    for (const auto other : allVehicles)
    {
        if (other == this)
            continue;

        // 检查目标车道上前后车辆的距离
        if (other->lane == tempTargetLane)
        {
            int distance = abs(x - other->x);
            int safeDist = getSafeDistance();
            if (distance < safeDist)
            {
                isGoing2change = false;
                laneChangeTrajectory.clear(); // 清空轨迹点
                return false;                 // 目标车道没有足够空间
            }
        }
    }

    // 变道安全，设置参数
    targetLane = tempTargetLane;
    startX = x;
    startY = y;
    endX = laneChangeTrajectory.back().first;  // 使用轨迹的终点
    endY = laneChangeTrajectory.back().second; // 使用轨迹的终点

    // 开始变道
    isChangingLane = true;
    changeProgress = 0.0f;
    return false;
}

void Vehicle::predictAndDrawTrajectory(QPainter &painter, int laneHeight, int middleY, int roadWidth, const std::vector<Vehicle *> &allVehicles) const
{
    VirtualVehicle virtualCar(x, y, carlength, carwidth);
    virtualCar.addTrajectoryPoint(x, y);

    int currentX = x;
    int currentY = y;
    int currentSpeed = (y < middleY) ? speed : -speed;
    const int predSteps = 30; // 统一预测步数

    // 如果正在变道，使用预计算的轨迹点
    if (isChangingLane)
    {
        // 使用已预计算的变道轨迹点
        for (size_t i = 1; i < laneChangeTrajectory.size(); ++i)
        {
            virtualCar.addTrajectoryPoint(laneChangeTrajectory[i].first, laneChangeTrajectory[i].second);
        }
    }
    else
    {
        // 预测直线行驶轨迹
        for (int i = 1; i <= predSteps; ++i)
        {
            int newX = currentX + i * currentSpeed * (1.0f / predSteps) * 15; // 调整水平位移计算
            virtualCar.addTrajectoryPoint(newX, currentY);
        }
    }

    bool useBlueColor = !isChangingLane && !isGoing2change;

    // 轨迹颜色
    QColor trajColor = useBlueColor ? QColor(0, 120, 215) : QColor(180, 180, 180);
    QPen oldPen = painter.pen();
    QPen trajPen(trajColor, 2);
    painter.setPen(trajPen);

    const auto &traj = virtualCar.trajectory;
    if (traj.size() >= 2)
    {
        for (size_t i = 1; i < traj.size(); ++i)
        {
            double x0 = traj[i - 1].first;
            double y0 = traj[i - 1].second;
            double x1 = traj[i].first;
            double y1 = traj[i].second;

            // 如果整段都在道路区域外，跳过
            if (x0 >= roadWidth && x1 >= roadWidth)
                continue;

            // 若线段部分越过道路边界，则计算交点并裁切
            if (x0 < roadWidth && x1 > roadWidth)
            {
                double t = (double)(roadWidth - x0) / (double)(x1 - x0);
                double newY1 = y0 + (y1 - y0) * t;
                x1 = roadWidth;
                y1 = (int)round(newY1);
            }
            else if (x0 > roadWidth && x1 < roadWidth)
            {
                double t = (double)(roadWidth - x1) / (double)(x0 - x1);
                double newY0 = y1 + (y0 - y1) * t;
                x0 = roadWidth;
                y0 = (int)round(newY0);
            }

            // 再次检查：若裁切后点相等或非法则跳过
            if ((int)round(x0) == (int)round(x1) && (int)round(y0) == (int)round(y1))
                continue;

            // 只绘制在道路区域内的线段
            painter.drawLine((int)round(x0), (int)round(y0), (int)round(x1), (int)round(y1));
        }
    }

    // 恢复画笔状态
    painter.setPen(oldPen);
}

void Vehicle::checkFrontVehicleDistance(std::vector<Vehicle *> &allVehicles, int safeDistance, int laneHeight, int stoppingSpeed)
{
    const int CRASH_DISTANCE = 10; // 碰撞距离
    const int WAIT = 5;            // 等待阈值
    const int CRASH = 15;          // 碰撞阈值

    for (auto other = allVehicles.begin(); other != allVehicles.end(); ++other)
    {
        if ((*other) == this)
            continue;

        // 只检查同车道车辆
        if ((*other)->lane != lane)
            continue;

        // 判断前车
        bool isFrontVehicle = false;
        if (lane < 3)
        {
            // 向右行驶，x坐标更大的车是前车
            isFrontVehicle = ((*other)->x > x);
        }
        else
        {
            // 向左行驶，x坐标更小的车是前车
            isFrontVehicle = ((*other)->x < x);
        }

        if (!isFrontVehicle)
            continue;

        // 计算两车之间的距离
        int distance = abs((*other)->x - x) - ((*other)->carlength / 2 + carlength / 2);

        if ((distance <= safeDistance) && (distance > CRASH_DISTANCE))
        {
            // 先计算相对速度以决定是否尝试变道
            int relativeSpeed = abs(speed - (*other)->speed);
            bool willAttempt = false;

            if (relativeSpeed <= WAIT)
            {
                if (((*other)->isBrokenDown))
                {
                    willAttempt = true;
                    isGoing2change = true;
                }
                else
                {
                    // 如果相对速度小于等于WAIT，将后车速度设为前车速度（减速）
                    speed = std::max(0, speed - stoppingSpeed);
                }
            }
            else if ((relativeSpeed > WAIT) && (relativeSpeed <= CRASH))
            {
                // 相对速度在可尝试变道区间
                willAttempt = true;
                isGoing2change = true;
            }

            // 标记为距离过近
            isTooClose = true;
            return;
        }
        else if (distance <= CRASH_DISTANCE)
        {
            // 非常近，处理危险情况
            (*other)->handleDangerousSituation();
            handleDangerousSituation();
            return;
        }
    }

    // 如果没有近距离车辆，取消警告状态
    isTooClose = false;
}

void Vehicle::showFlashingFrame(QPainter &painter, int roadWidth, bool isAttempting) const
{
    // 保存当前画笔和画刷状态
    QPen oldPen = painter.pen();
    QBrush oldBrush = painter.brush();
    
    // 尝试变道时使用蓝色框；否则使用橘色（接近/警告）
    QColor frameColor = isAttempting ? QColor(0, 120, 215) : QColor(255, 165, 0); // 蓝色或橘色

    QPen framePen(frameColor, 2);
    painter.setPen(framePen);
    painter.setBrush(Qt::NoBrush); // 设置为透明填充，确保能看到车辆本身

    // 检查车辆是否接近或进入控制栏区域
    if (x + carlength / 2 + 5 <= roadWidth)
    {
        // 车辆完全在控制栏左侧，正常绘制
        painter.drawRect(x - carlength / 2 - 5, y - carwidth / 2 - 5,
                         carlength + 10, carwidth + 10);
    }
    else if (x - carlength / 2 - 5 < roadWidth)
    {
        // 车辆部分进入控制栏区域，只绘制未进入控制栏的部分
        painter.drawRect(x - carlength / 2 - 5, y - carwidth / 2 - 5,
                         roadWidth - (x - carlength / 2 - 5), carwidth + 10);
    }
    // 如果车辆完全在控制栏内，则不绘制任何框

    // 恢复画笔和画刷状态
    painter.setPen(oldPen);
    painter.setBrush(oldBrush);
}

void Vehicle::handleDangerousSituation()
{
    isBrokenDown = true;
    speed = 0;
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
    QColor backgroundColor = getBackgroundColor();
    painter.fillRect(0, topBarHeight, roadWidth, windowHeight - topBarHeight, backgroundColor);

    // 绘制下雨或下雪效果
    if (currentWeather == RAIN || currentWeather == SNOW) {
        drawWeatherEffect(painter, backgroundColor);
    }

    // 绘制车道线
    painter.setPen(QPen(Qt::white, 1, Qt::DashLine));
    int laneCount = 6;
    int laneHeight = (windowHeight - topBarHeight) / laneCount;
    for (int i = 0; i < laneCount - 1; ++i)
    {
        int y = topBarHeight + (i + 1) * laneHeight;
        painter.drawLine(0, y, roadWidth, y);
    }

    // 绘制车辆和轨迹预测
    int middleY = (windowHeight + topBarHeight) / 2;
    for (const auto &v : vehicles)
    {
        // 绘制轨迹预测
        v->predictAndDrawTrajectory(painter, laneHeight, middleY, roadWidth, vehicles);

        // 绘制车辆
        v->draw(painter);

        // 绘制警告框
        if (v->isTooClose)
        {
            v->showFlashingFrame(painter, roadWidth, v->isGoing2change);
        }
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
        int carlength = (int)(normallength(rng) * scale * 1.5); // 增加车辆长度

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
            newVehicle = new Sedan(lane, carlength, carwidth, newX, newY, speed);
        }
        else if (vehicleType == 1)
        {
            newVehicle = new SUV(lane, carlength, carwidth, newX, newY, speed);
        }
        else
        {
            newVehicle = new Truck(lane, carlength, carwidth, newX, newY, speed);
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

    // 检查车辆之间距离并更新状态
    for (size_t i = 0; i < vehicles.size(); ++i)
    {
        Vehicle *v = vehicles[i];

        // 检查与其他车辆的距离
        v->isTooClose = false;
        v->checkFrontVehicleDistance(vehicles, safeDistance, laneHeight, stoppingSpeed);

        // 处理变道逻辑（仅对部分车辆启用）
        if (!v->isChangingLane && !v->isGoing2change && (rng() % 1000) < 5) // 0.5%概率尝试变道
        {
            v->isGoing2change = true;
        }

        // 执行变道或正常行驶
        if (v->isGoing2change)
        {
            v->smoothLaneChange(laneHeight, vehicles);
        }
        else if (v->isChangingLane)
        {
            v->smoothLaneChange(laneHeight, vehicles);
        }
        else
        {
            // 简化的车辆移动逻辑
            v->moveForward(middleY);
        }
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
        // 白天根据天气设置背景色
        switch (currentWeather)
        {
        case RAIN:
            return QColor(150, 150, 160); // 雨天的灰蓝色
        case SNOW:
            return QColor(200, 200, 210); // 雪天的灰白色
        default:
            return QColor(100, 100, 100); // 晴天为灰色
        }
    }
    else
    {
        // 夜晚根据天气设置背景色
        switch (currentWeather)
        {
        case RAIN:
            return QColor(30, 30, 40); // 雨夜为深蓝灰色
        case SNOW:
            return QColor(40, 40, 50); // 雪夜为深灰白色
        default:
            return QColor(0, 0, 0); // 晴夜为黑色
        }
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
    QColor rainColor(150, 150, 200);  // 雨天按钮颜色
    QColor snowColor(200, 200, 220);  // 雪天按钮颜色

    QColor normalBtnColor = (currentWeather == NOTHING) ? activeColor : inactiveColor;
    QColor rainBtnColor = (currentWeather == RAIN) ? rainColor : inactiveColor;
    QColor snowBtnColor = (currentWeather == SNOW) ? snowColor : inactiveColor;

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

void QtWidgetsApplication1::drawWeatherEffect(QPainter &painter, const QColor &backgroundColor)
{
    // 保存当前画笔状态
    painter.save();
    
    // 设置随机数生成器
    static std::mt19937 weatherRng(QDateTime::currentMSecsSinceEpoch());
    static std::uniform_real_distribution<float> positionDist(0.0, 1.0);
    static std::uniform_real_distribution<float> speedDist(0.5, 2.0);
    
    // 根据天气类型绘制效果
    if (currentWeather == RAIN) {
        // 绘制雨滴效果
        painter.setPen(QPen(QColor(180, 180, 220, 150), 1));
        
        // 绘制一定数量的雨滴
        int raindropCount = 200;
        for (int i = 0; i < raindropCount; ++i) {
            float x = positionDist(weatherRng) * roadWidth;
            float y = positionDist(weatherRng) * (windowHeight - topBarHeight) + topBarHeight;
            
            // 添加一些动态效果
            float speed = speedDist(weatherRng);
            float length = 10.0f * speed;
            
            // 绘制雨滴（倾斜的短线）
            painter.drawLine(x, y, x - 3, y + length);
        }
    }
    else if (currentWeather == SNOW) {
        // 绘制雪花效果
        painter.setPen(QPen(QColor(230, 230, 230, 200), 1));
        
        // 绘制一定数量的雪花
        int snowflakeCount = 150;
        for (int i = 0; i < snowflakeCount; ++i) {
            float x = positionDist(weatherRng) * roadWidth;
            float y = positionDist(weatherRng) * (windowHeight - topBarHeight) + topBarHeight;
            
            // 绘制雪花（小圆点）
            painter.drawEllipse(QPointF(x, y), 1.5, 1.5);
        }
    }
    
    // 恢复画笔状态
    painter.restore();
}
