#include <graphics.h>
#include <vector>
#include <ctime>
#include <conio.h>
#include <Windows.h>
#include <sstream>
#include <string>
#include <iostream>
#include <memory>

#include "Random.h"
#include "Class.h"
#include "Define.h"
using namespace std;

// 平滑变道函数
bool Vehicle::smoothLaneChange(int laneHeight, const vector<unique_ptr<Vehicle>> &allVehicles)
{
    // 如果车辆已抛锚，不能变道
    if (isBrokenDown)
    {
        return false;
    }

    if (isChangingLane)
    {
        // 更新变道进度
        changeProgress += 0.02f;

        if (changeProgress >= 1.0f)
        {
            // 变道完成
            changeProgress = 1.0f;
            isChangingLane = false;
            isGoing2change = false;
            lane = targetLane;
            speed = speed * 2; // 恢复速度
            return true;
        }

        // 使用固定的渐入渐出贝塞尔曲线计算垂直方向速度
        float t = changeProgress;
        float verticalSpeed = 3 * t * t - 2 * t * t * t;

        // 计算垂直方向上的位置变化
        float deltaY = (endY - startY) * verticalSpeed;
        y = startY + (int)deltaY;

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

    // 创建虚拟车辆用于轨迹预测
    VirtualVehicle virtualCar(x, y, carlength, carwidth);
    virtualCar.addTrajectoryPoint(x, y);

    // 预测变道轨迹
    int currentX = x;
    int currentY = y;
    int targetY = laneHeight * tempTargetLane + (int)(0.5 * laneHeight);
    int currentSpeed = (y < laneHeight * 3) ? speed : -speed;

    for (int i = 1; i <= 30; ++i)
    {
        float t = min(1.0f, i * 0.02f);
        float verticalSpeed = 3 * t * t - 2 * t * t * t;
        float deltaY = (targetY - currentY) * verticalSpeed;
        int newY = currentY + (int)deltaY;
        int newX = currentX + i * currentSpeed;
        virtualCar.addTrajectoryPoint(newX, newY);
    }

    // 检查与其他车辆的轨迹是否相交
    for (const auto &other : allVehicles)
    {
        if (other.get() == this)
            continue;

        VirtualVehicle otherVirtual(other->x, other->y, other->carlength, other->carwidth);

        if (other->isChangingLane)
        {
            float otherProgress = other->changeProgress;
            int otherStartY = other->startY;
            int otherEndY = other->endY;
            int otherSpeed = (other->y < laneHeight * 3) ? other->speed : -other->speed;

            for (int i = 1; i <= 30; ++i)
            {
                float t = min(1.0f, otherProgress + i * 0.02f);
                float verticalSpeed = 3 * t * t - 2 * t * t * t;
                float deltaY = (otherEndY - otherStartY) * verticalSpeed;
                int newY = otherStartY + (int)deltaY;
                int newX = other->x + i * otherSpeed;
                otherVirtual.addTrajectoryPoint(newX, newY);
            }
        }
        else
        {
            int otherSpeed = (other->y < laneHeight * 3) ? other->speed : -other->speed;
            for (int i = 1; i <= 30; ++i)
            {
                int newX = other->x + i * otherSpeed;
                otherVirtual.addTrajectoryPoint(newX, other->y);
            }
        }

        if (virtualCar.isTrajectoryIntersecting(otherVirtual, 30))
        {
            isGoing2change = false;
            return false;
        }
    }

    // 变道安全，设置参数
    targetLane = tempTargetLane;
    startX = x;
    startY = y;
    endX = x + 25;
    endY = laneHeight * targetLane + (int)(0.5 * laneHeight);
    isChangingLane = true;
    changeProgress = 0.0f;
    return false;
}

// 预测并绘制轨迹
void Vehicle::predictAndDrawTrajectory(int laneHeight, int middleY, int predictionSteps, const vector<unique_ptr<Vehicle>> &allVehicles) const
{
    VirtualVehicle virtualCar(x, y, carlength, carwidth);
    virtualCar.addTrajectoryPoint(x, y);

    int currentX = x;
    int currentY = y;
    int currentSpeed = (y < middleY) ? speed : -speed;

    if (isChangingLane)
    {
        float currentProgress = changeProgress;
        int currentStartY = startY;
        int currentEndY = endY;

        for (int i = 1; i <= predictionSteps; ++i)
        {
            float t = min(1.0f, currentProgress + i * 0.02f);
            float verticalSpeed = 3 * t * t - 2 * t * t * t;
            float deltaY = (currentEndY - currentStartY) * verticalSpeed;
            int newY = currentStartY + (int)deltaY;
            int newX = currentX + i * currentSpeed;
            virtualCar.addTrajectoryPoint(newX, newY);
        }
    }
    else
    {
        for (int i = 1; i <= predictionSteps; ++i)
        {
            int newX = currentX + i * currentSpeed;
            virtualCar.addTrajectoryPoint(newX, currentY);
        }
    }

    bool isSafe = true;
    for (const auto &other : allVehicles)
    {
        if (other.get() == this)
            continue;

        VirtualVehicle otherVirtual(other->x, other->y, other->carlength, other->carwidth);

        int otherSpeed = (other->y < middleY) ? other->speed : -other->speed;
        for (int i = 1; i <= predictionSteps; ++i)
        {
            int newX = other->x + i * otherSpeed;
            otherVirtual.addTrajectoryPoint(newX, other->y);
        }

        if (virtualCar.isTrajectoryIntersecting(otherVirtual, predictionSteps))
        {
            isSafe = false;
            break;
        }
    }

    bool useBlueColor = !isChangingLane && !isGoing2change;
    virtualCar.drawTrajectory(useBlueColor);
}

// 检查变道是否安全
bool Vehicle::isLaneChangeSafe(int laneHeight, const vector<unique_ptr<Vehicle>> &allVehicles) const
{
    if (haschanged)
    {
        return true;
    }

    int target = 0;
    if (lane == 0 || lane == 3)
    {
        target = lane + 1;
    }
    else if (lane == 2 || lane == 5)
    {
        target = lane - 1;
    }
    else if (lane == 1 || lane == 4)
    {
        target = lane + (rand() % 2 ? 1 : -1);
    }

    VirtualVehicle virtualCar(x, y, carlength, carwidth);
    virtualCar.addTrajectoryPoint(x, y);

    int currentX = x;
    int currentY = y;
    int targetY = laneHeight * target + (int)(0.5 * laneHeight);
    int currentSpeed = (y < laneHeight * 3) ? speed : -speed;

    for (int i = 1; i <= 30; ++i)
    {
        float t = min(1.0f, i * 0.02f);
        float verticalSpeed = 3 * t * t - 2 * t * t * t;
        float deltaY = (targetY - currentY) * verticalSpeed;
        int newY = currentY + (int)deltaY;
        int newX = currentX + i * currentSpeed;
        virtualCar.addTrajectoryPoint(newX, newY);
    }

    for (const auto &other : allVehicles)
    {
        if (other.get() == this)
            continue;

        VirtualVehicle otherVirtual(other->x, other->y, other->carlength, other->carwidth);

        if (other->isChangingLane)
        {
            float otherProgress = other->changeProgress;
            int otherStartY = other->startY;
            int otherEndY = other->endY;
            int otherSpeed = (other->y < laneHeight * 3) ? other->speed : -other->speed;

            for (int i = 1; i <= 30; ++i)
            {
                float t = min(1.0f, otherProgress + i * 0.02f);
                float verticalSpeed = 3 * t * t - 2 * t * t * t;
                float deltaY = (otherEndY - otherStartY) * verticalSpeed;
                int newY = otherStartY + (int)deltaY;
                int newX = other->x + i * otherSpeed;
                otherVirtual.addTrajectoryPoint(newX, newY);
            }
        }
        else
        {
            int otherSpeed = (other->y < laneHeight * 3) ? other->speed : -other->speed;
            for (int i = 1; i <= 30; ++i)
            {
                int newX = other->x + i * otherSpeed;
                otherVirtual.addTrajectoryPoint(newX, other->y);
            }
        }

        if (virtualCar.isTrajectoryIntersecting(otherVirtual, 30))
        {
            return false;
        }
    }

    return true;
}

// 检查与前车距离
void Vehicle::checkFrontVehicleDistance(vector<unique_ptr<Vehicle>> &allVehicles, int safeDistance)
{
    for (auto &other : allVehicles)
    {
        if (other.get() == this)
            continue;

        if (other->lane != lane)
            continue;

        bool isMovingRight = (lane < 3);
        bool isFrontVehicle = false;
        
        if (isMovingRight)
        {
            isFrontVehicle = (other->x > x);
        }
        else
        {
            isFrontVehicle = (other->x < x);
        }

        if (!isFrontVehicle)
            continue;

        int distance = abs(other->x - x) - (other->carlength / 2 + carlength / 2);

        if ((distance <= safeDistance) && (distance > CRASH_DISTANCE))
        {
            showFlashingFrame();
            int relativeSpeed = abs(speed - other->speed);
            if (relativeSpeed != 0)
            {
                cout << "Relative Speed: " << relativeSpeed << endl;
            }
            
            if (relativeSpeed <= WAIT)
            {
                if (relativeSpeed > WAIT / 2)
                {
                    speed = speed - 5;
                }
                else
                {
                    speed = other->speed;
                }
            }
            if (relativeSpeed > WAIT && relativeSpeed <= CRASH)
            {
                isGoing2change = true;
                if (other->speed != 0)
                {
                    speed = speed / 2;
                }
            }
            return;
        }
        else if (distance <= CRASH_DISTANCE)
        {
            showFlashingFrame();
            int relativeSpeed = abs(speed - other->speed);
            if (relativeSpeed != 0)
            {
                cout << "Relative Speed:CRASH " << relativeSpeed << endl;
            }
            other->handleDangerousSituation();
            handleDangerousSituation();
            return;
        }
    }
}

// 显示橘色线框
void Vehicle::showFlashingFrame()
{
    LINESTYLE oldLineStyle;
    getlinestyle(&oldLineStyle);
    COLORREF oldLineColor = getlinecolor();

    setlinecolor(RGB(255, 165, 0));
    setlinestyle(PS_SOLID, 2);

    rectangle(x - carlength / 2 - 5, y - carwidth / 2 - 5,
              x + carlength / 2 + 5, y + carwidth / 2 + 5);

    setlinestyle(oldLineStyle.style, oldLineStyle.thickness);
    setlinecolor(oldLineColor);
}

// 处理危险情况
void Vehicle::handleDangerousSituation()
{
    isBrokenDown = true;
    speed = 0;
}
