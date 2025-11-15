#include <graphics.h>
#include <vector>
#include <ctime>
#include <conio.h> // 需要包含此头文件_kbhit()函数需要
#include <Windows.h>
#include <sstream>
#include <string>
#include <iostream>

#include "Random.h"
#include "Class.h"
#include "Define.h"
#include "VehicleTypes.h"
using namespace std;

// 绘制变道轨迹（红色虚线）
// 平滑变道函数
bool Vehicle::smoothLaneChange(int laneHeight, const vector<Vehicle *> &allVehicles)
{
    // 如果车辆已抛锚，不能变道
    if (isBrokenDown)
    {
        return false;
    }

    if (isChangingLane)
    {
        // 更新变道进度，基于车辆类型调整变道速度
        // 小轿车变道较快，卡车较慢
        float changeSpeed = 0.02f;
        if (dynamic_cast<const Sedan*>(this)) {
            changeSpeed = 0.025f;  // 小轿车变道稍快
        } else if (dynamic_cast<const Truck*>(this)) {
            changeSpeed = 0.015f;  // 卡车变道稍慢
        }
        
        changeProgress += changeSpeed;

        if (changeProgress >= 1.0f)
        {
            // 变道完成
            changeProgress = 1.0f;
            isChangingLane = false;
            isGoing2change = false;
            lane = targetLane;
            speed = speed * 2; // 恢复速度
            
            // 清空轨迹点
            laneChangeTrajectory.clear();
            return true;
        }

        // 使用预计算的轨迹点更新车辆位置
        int trajectoryIndex = (int)(changeProgress * (laneChangeTrajectory.size() - 1));
        if (trajectoryIndex >= laneChangeTrajectory.size()) {
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
    
    // 垂直方向目标位置
    int currentY = y;
    int targetY = laneHeight * tempTargetLane + (int)(0.5 * laneHeight);
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
        laneChangeTrajectory.push_back(make_pair(newX, newY));
    }

    // 创建虚拟车辆用于轨迹预测
    VirtualVehicle virtualCar(x, y, carlength, carwidth);

    // 添加轨迹点用于碰撞检测
    for (const auto& point : laneChangeTrajectory) {
        virtualCar.addTrajectoryPoint(point.first, point.second);
    }

    // 检查与其他车辆的轨迹是否相交
    for (const auto other : allVehicles)
    {
        if (other == this)
            continue; // 跳过自己

        // 为其他车辆创建虚拟车辆
        VirtualVehicle otherVirtual((*other).x, (*other).y, (*other).carlength, (*other).carwidth);

        // 判断其他车辆是否在变道中
        if ((*other).isChangingLane)
        {
            // 如果其他车辆也在变道，预测其变道轨迹
            float otherProgress = (*other).changeProgress;
            int otherStartX = (*other).startX;
            int otherStartY = (*other).startY;
            int otherEndX = (*other).endX;
            int otherEndY = (*other).endY;
            int otherSpeed = ((*other).y < laneHeight * 3) ? (*other).speed : -(*other).speed;

            // 预测其他车辆的变道轨迹
            for (int i = 1; i <= predictionSteps; ++i)
            {
                // 更新进度
                float t = min(1.0f, otherProgress + i * (1.0f / predictionSteps));

                // 计算垂直位置 使用该车辆特定的变道曲线函数
                float verticalSpeed = other->curveFunc(t);
                float deltaY = (otherEndY - otherStartY) * verticalSpeed;
                int newY = otherStartY + (int)deltaY;

                // 计算水平位置（保持原有速度）
                int newX = (*other).x + i * otherSpeed * (1.0f / predictionSteps) * 15; // 调整水平位移计算

                // 添加到轨迹
                otherVirtual.addTrajectoryPoint(newX, newY);
            }
        }
        else
        {
            // 其他车辆直线行驶，预测其直线轨迹
            int otherSpeed = ((*other).y < laneHeight * 3) ? (*other).speed : -(*other).speed;
            for (int i = 1; i <= predictionSteps; ++i)
            {
                int newX = (*other).x + i * otherSpeed * (1.0f / predictionSteps) * 15; // 调整水平位移计算
                otherVirtual.addTrajectoryPoint(newX, (*other).y);
            }
        }

        // 检查轨迹是否相交
        if (virtualCar.isTrajectoryIntersecting(otherVirtual, predictionSteps))
        {
            isGoing2change = false; // 取消准备变道状态
            laneChangeTrajectory.clear(); // 清空轨迹点
            return false;           // 轨迹相交，变道不安全，取消变道
        }
    }

    // 变道安全，设置目标车道和变道参数
    targetLane = tempTargetLane;
    startX = x;
    startY = y;
    endX = laneChangeTrajectory.back().first; // 使用轨迹的终点
    endY = laneChangeTrajectory.back().second; // 使用轨迹的终点

    // 开始变道
    isChangingLane = true;
    changeProgress = 0.0f;
    return false;
}

// 预测并绘制轨迹
void Vehicle::predictAndDrawTrajectory(int laneHeight, int middleY, int predictionSteps, const vector<Vehicle *> &allVehicles) const
{
    // 创建虚拟车辆
    VirtualVehicle virtualCar(x, y, carlength, carwidth);

    // 添加当前位置
    virtualCar.addTrajectoryPoint(x, y);

    // 预测未来轨迹
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

    // 检查与其他车辆的轨迹是否相交
    bool isSafe = true;
    for (const auto other : allVehicles)
    {
        if (other == this)
            continue; // 跳过自己

        // 为其他车辆创建虚拟车辆
        VirtualVehicle otherVirtual((*other).x, (*other).y, (*other).carlength, (*other).carwidth);

        // 预测其他车辆的直线行驶轨迹
        int otherSpeed = ((*other).y < middleY) ? (*other).speed : -(*other).speed;
        for (int i = 1; i <= predSteps; ++i)
        {
            int newX = (*other).x + i * otherSpeed * (1.0f / predSteps) * 15; // 调整水平位移计算
            otherVirtual.addTrajectoryPoint(newX, (*other).y);
        }

        // 检查轨迹是否相交
        if (virtualCar.isTrajectoryIntersecting(otherVirtual, predSteps))
        {
            isSafe = false;
            break;
        }
    }

    // 绘制轨迹，根据车辆状态使用不同颜色
    // 直行时使用蓝色，准备变道或正在变道时使用红色
    bool useBlueColor = !isChangingLane && !isGoing2change;
    virtualCar.drawTrajectory(useBlueColor);
}

// 检查变道是否安全
bool Vehicle::isLaneChangeSafe(int laneHeight, const vector<Vehicle *> &allVehicles) const
{
    // 如果已经变道或不在变道点，返回安全
    if (haschanged)
    {
        return true;
    }

    // 确定目标车道
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

    // 创建虚拟车辆用于轨迹预测
    VirtualVehicle virtualCar(x, y, carlength, carwidth);

    // 添加当前位置
    virtualCar.addTrajectoryPoint(x, y);

    // 预测变道轨迹
    int currentX = x;
    int currentY = y;
    int targetY = laneHeight * target + (int)(0.5 * laneHeight);
    int currentSpeed = (y < laneHeight * 3) ? speed : -speed;
    const int predictionSteps = 30; // 统一预测步数

    // 预测变道轨迹
    for (int i = 1; i <= predictionSteps; ++i)
    {
        // 计算进度
        float t = min(1.0f, i * (1.0f / predictionSteps));

        // 计算垂直位置
        float verticalSpeed = curveFunc(t);
        float deltaY = (targetY - currentY) * verticalSpeed;
        int newY = currentY + (int)deltaY;

        // 计算水平位置（保持原有速度）
        int newX = currentX + i * currentSpeed * (1.0f / predictionSteps) * 15; // 调整水平位移计算

        // 添加到轨迹
        virtualCar.addTrajectoryPoint(newX, newY);
    }

    // 检查与其他车辆的轨迹是否相交
    for (const auto other : allVehicles)
    {
        if (other == this)
            continue; // 跳过自己

        // 为其他车辆创建虚拟车辆
        VirtualVehicle otherVirtual((*other).x, (*other).y, (*other).carlength, (*other).carwidth);

        // 判断其他车辆是否在变道中
        if ((*other).isChangingLane)
        {
            // 如果其他车辆也在变道，预测其变道轨迹
            float otherProgress = (*other).changeProgress;
            int otherStartX = (*other).startX;
            int otherStartY = (*other).startY;
            int otherEndX = (*other).endX;
            int otherEndY = (*other).endY;
            int otherSpeed = ((*other).y < laneHeight * 3) ? (*other).speed : -(*other).speed;

            // 预测其他车辆的变道轨迹
            for (int i = 1; i <= predictionSteps; ++i)
            {
                // 更新进度
                float t = min(1.0f, otherProgress + i * (1.0f / predictionSteps));

                // 计算垂直位置
                float verticalSpeed = other->curveFunc(t);
                float deltaY = (otherEndY - otherStartY) * verticalSpeed;
                int newY = otherStartY + (int)deltaY;

                // 计算水平位置（保持原有速度）
                int newX = (*other).x + i * otherSpeed * (1.0f / predictionSteps) * 15; // 调整水平位移计算

                // 添加到轨迹
                otherVirtual.addTrajectoryPoint(newX, newY);
            }
        }
        else
        {
            // 其他车辆直线行驶，预测其直线轨迹
            int otherSpeed = ((*other).y < laneHeight * 3) ? (*other).speed : -(*other).speed;
            for (int i = 1; i <= predictionSteps; ++i)
            {
                int newX = (*other).x + i * otherSpeed * (1.0f / predictionSteps) * 15; // 调整水平位移计算
                otherVirtual.addTrajectoryPoint(newX, (*other).y);
            }
        }

        // 检查轨迹是否相交
        if (virtualCar.isTrajectoryIntersecting(otherVirtual, predictionSteps))
        {
            return false; // 轨迹相交，变道不安全
        }
    }

    return true; // 轨迹不相交，变道安全
}

// 检查与前车距离
void Vehicle::checkFrontVehicleDistance(vector<Vehicle *> &allVehicles, int safeDistance)
{
    // 遍历所有车辆，寻找同一车道的前方车辆
    for (auto other : allVehicles)
    {
        // 跳过自己
        if (other == this)
            continue;

        // 检查是否在同一车道
        if ((*other).lane != lane)
            continue;

        // 判断方向：上方车道(0,1,2)向右行驶，下方车道(3,4,5)向左行驶
        bool isMovingRight = (lane < 3);

        // 检查是否为前方车辆
        bool isFrontVehicle = false;
        if (isMovingRight)
        {
            // 向右行驶，x坐标更大的车是前车
            isFrontVehicle = ((*other).x > x);
        }
        else
        {
            // 向左行驶，x坐标更小的车是前车
            isFrontVehicle = ((*other).x < x);
        }

        if (!isFrontVehicle)
            continue;

        // 计算两车之间的距离
        int distance = abs((*other).x - x) - ((*other).carlength / 2 + carlength / 2);

        // 如果距离小于等于安全距离，进行进一步处理
        if ((distance <= safeDistance) && (distance > CRASH_DISTANCE))
        {
            showFlashingFrame();
            // 计算相对速度
            int relativeSpeed = abs(speed - (*other).speed);
            if (relativeSpeed != 0)
            {
                cout << "Relative Speed: " << relativeSpeed << endl;
            }
            // 根据相对速度采取不同措施
            if (relativeSpeed <= WAIT)
            {
                // 如果相对速度小于等于WAIT，将后车速度设为前车速度
                if (relativeSpeed > WAIT / 2)
                {
                    speed = speed - 5;
                }
                else
                {
                    speed = (*other).speed;
                }
            }
            if (relativeSpeed > WAIT && relativeSpeed <= CRASH)
            {
                isGoing2change = true;
                if ((*other).speed != 0)
                {
                    speed = speed / 2;
                }
            }
            return; // 找到最近的前车后即可返回
        }
        else if (distance <= CRASH_DISTANCE)
        {
            showFlashingFrame();
            // 计算相对速度
            int relativeSpeed = abs(speed - (*other).speed);
            if (relativeSpeed != 0)
            {
                cout << "Relative Speed:CRASH " << relativeSpeed << endl;
            }
            (*other).handleDangerousSituation();
            handleDangerousSituation();
            return; // 找到最近的前车后即可返回
        }
    }
}

// 显示橘色线框
void Vehicle::showFlashingFrame()
{
    // 保存当前线型和颜色
    LINESTYLE oldLineStyle;
    getlinestyle(&oldLineStyle);
    COLORREF oldLineColor = getlinecolor();

    // 设置橘色线框
    setlinecolor(RGB(255, 165, 0)); // 橙色
    setlinestyle(PS_SOLID, 2);      // 实线，线宽为2

    // 绘制车辆周围的橘色线框
    rectangle(x - carlength / 2 - 5, y - carwidth / 2 - 5,
              x + carlength / 2 + 5, y + carwidth / 2 + 5);

    // 恢复原来的线型和颜色
    setlinestyle(oldLineStyle.style, oldLineStyle.thickness);
    setlinecolor(oldLineColor);
}

// 处理危险情况
void Vehicle::handleDangerousSituation()
{
    // 设置车辆为抛锚状态
    isBrokenDown = true;
    // 将车辆速度设为0
    speed = 0;
}
