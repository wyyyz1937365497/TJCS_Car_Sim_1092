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
#include "VehicleTypes.h"
using namespace std;

// 平滑变道函数
bool Vehicle::smoothLaneChange(int laneHeight, const vector<Vehicle *> &allVehicles)
{
    cout << "正在平滑变道..." << endl;
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
        laneChangeTrajectory.push_back(make_pair(newX, newY));
    }

    // 创建虚拟车辆用于轨迹预测
    VirtualVehicle virtualCar(x, y, carlength, carwidth);

    // 添加轨迹点用于碰撞检测
    for (const auto &point : laneChangeTrajectory)
    {
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
            cout << "变道失败" << endl;
            isGoing2change = false;       // 取消准备变道状态
            laneChangeTrajectory.clear(); // 清空轨迹点
            return false;                 // 轨迹相交，变道不安全，取消变道
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

// 预测并绘制轨迹
void Vehicle::predictAndDrawTrajectory(int laneHeight, int middleY, int predictionSteps, const vector<Vehicle *> &allVehicles) const
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

    bool useBlueColor = !isChangingLane && !isGoing2change;

    // --- 替换原先直接调用 virtualCar.drawTrajectory 的行为 ---
    // 在这里我们自己绘制轨迹线段，并确保不在右侧 control bar（宽度 60）上绘制。
    // 采用与 UI 一致的右侧 control bar 宽度 60（Car_Sim.cpp 中定义）作为屏蔽区域宽度。
    const int controlBarWidth = 60;
    int rightLimit = getwidth() - controlBarWidth; // 轨迹绘制的最大 x 值（不允许超过此值绘制到 control bar）

    // 轨迹颜色
    COLORREF trajColor = useBlueColor ? RGB(0, 120, 215) : RGB(180, 180, 180);
    COLORREF oldColor = getlinecolor();
    LINESTYLE oldStyle;
    getlinestyle(&oldStyle);

    setlinecolor(trajColor);
    setlinestyle(PS_SOLID, 2);

    const auto &traj = virtualCar.trajectory;
    if (traj.size() >= 2)
    {
        for (size_t i = 1; i < traj.size(); ++i)
        {
            double x0 = traj[i - 1].first;
            double y0 = traj[i - 1].second;
            double x1 = traj[i].first;
            double y1 = traj[i].second;

            // 如果整段都在 control bar 右侧，跳过
            if (x0 >= rightLimit && x1 >= rightLimit)
                continue;

            // 若线段部分越过 rightLimit，则计算交点并裁切
            if (x0 < rightLimit && x1 > rightLimit)
            {
                double t = (double)(rightLimit - x0) / (double)(x1 - x0);
                double newY1 = y0 + (y1 - y0) * t;
                x1 = rightLimit;
                y1 = (int)round(newY1);
            }
            else if (x0 > rightLimit && x1 < rightLimit)
            {
                double t = (double)(rightLimit - x1) / (double)(x0 - x1);
                double newY0 = y1 + (y0 - y1) * t;
                x0 = rightLimit;
                y0 = (int)round(newY0);
            }

            // 再次检查：若裁切后点相等或非法则跳过
            if ((int)round(x0) == (int)round(x1) && (int)round(y0) == (int)round(y1))
                continue;

            // 只绘制在道路区域内的线段
            line((int)round(x0), (int)round(y0), (int)round(x1), (int)round(y1));
        }
    }

    // 恢复画笔状态
    setlinestyle(oldStyle.style, oldStyle.thickness);
    setlinecolor(oldColor);
}

// 检查变道是否安全
bool Vehicle::isLaneChangeSafe(int laneHeight, const vector<Vehicle *> &allVehicles) const
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
    int targetY = 80 + laneHeight * target + (int)(0.5 * laneHeight); // 添加80像素的顶部面板偏移
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
            return false;
        }
    }

    return true;
}

// 检查与前车距离
void Vehicle::checkFrontVehicleDistance(vector<Vehicle *> &allVehicles, int safeDistance, int laneHeight)
{
    if (isBrokenDown)
    {
        return;
    }
    // 遍历所有车辆，寻找同一车道的前方车辆
    for (auto other : allVehicles)
    {
        // 跳过自己
        if (other == this)
            continue;

        // 检查是否在同一车道
        if ((*other).lane != lane)
            continue;

        bool isMovingRight = (lane < 3);
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

        if ((distance <= safeDistance) && (distance > CRASH_DISTANCE))
        {
            // 先计算相对速度以决定是否尝试变道
            int relativeSpeed = abs(speed - (*other).speed);
            bool willAttempt = false;

            if (relativeSpeed <= WAIT)
            {
                if (((*other).isBrokenDown))
                {
                    willAttempt = true;
                    isGoing2change = true;
                }
                else
                {
                    // 如果相对速度小于等于WAIT，将后车速度设为前车速度（减速）
                    speed = speed - stoppingSpeed;
                }
            }
            else if ((relativeSpeed > WAIT) && (relativeSpeed <= CRASH))
            {
                // 相对速度在可尝试变道区间
                willAttempt = true;
                isGoing2change = true;
            }

            // 根据上面判断绘制闪烁框（蓝色表示尝试变道，橘色表示普通接近警告）
            showFlashingFrame(willAttempt);

            return;
        }
        else if (distance <= CRASH_DISTANCE)
        {
            // 非常近，显示橘色框并处理危险情况
            showFlashingFrame(false);
            (*other).handleDangerousSituation();
            handleDangerousSituation();
            return;
        }
    }
}

// 显示橘色/蓝色线框
void Vehicle::showFlashingFrame(bool isAttempting)
{
    LINESTYLE oldLineStyle;
    getlinestyle(&oldLineStyle);
    COLORREF oldLineColor = getlinecolor();

    // 尝试变道时使用蓝色框；否则使用橘色（接近/警告）
    if (isAttempting)
        setlinecolor(RGB(0, 120, 215)); // 蓝色
    else
        setlinecolor(RGB(255, 165, 0)); // 橘色

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
