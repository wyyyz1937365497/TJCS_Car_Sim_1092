#include <graphics.h>
#include <vector>
#include <ctime>
#include <conio.h>
#include <Windows.h>
#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>

#include "Random.h"
#include "Class.h"
#include "Define.h"
#include "VehicleTypes.h"
using namespace std;

int main()
{
    Bridge bridge;
    bridge.bridgeLength = 100;
    bridge.bridgeWidth = 50;
    bridge.widthScale = 1;

    int windowWidth, windowHeight;
    double scale;
    bridge.calculateWindowSize(windowWidth, windowHeight, scale);

    vector<Vehicle *> vehicles;
    srand(unsigned int(time(0)));
    double time = 0;

    normal_distribution<> normalwidth(3, 0.1);
    normal_distribution<> normallength(6, 0.1);
    uniform_int_distribution<int> int_dist(20, 120);
    RandomGenerator rng(int_dist);

    while (!_kbhit())
    {
        cleardevice();

        // 显示桥的参数信息
        wchar_t info[256];
        swprintf_s(info, L"桥长： %.0fm  桥宽：%.0fm  桥宽放大率： %.1f",
                   bridge.bridgeLength, bridge.bridgeWidth, bridge.widthScale);
        settextstyle(20, 0, L"Arial");
        outtextxy(10, 10, info);

        // 显示时间
        wchar_t info2[256];
        swprintf_s(info2, L"时间： %.0fs", time);
        settextstyle(20, 0, L"Arial");
        outtextxy(windowWidth - 150, 10, info2);

        // 绘制车道
        setlinecolor(WHITE);
        settextcolor(WHITE);
        int laneCount = 6;
        int laneHeight = (int)(windowHeight / laneCount);
        for (int i = 0; i < laneCount - 1; ++i)
        {
            drawDashedLine(0, (i + 1) * laneHeight, windowWidth, (i + 1) * laneHeight);
        }

        // 绘制箭头和可视化按钮
        for (int i = 0; i < laneCount; ++i)
        {
            int buttonX = 5;
            int buttonY = laneHeight * i + (int)(0.5 * laneHeight) - (int)(laneHeight / 4);
            int buttonWidth = 40;
            int buttonHeight = (int)(laneHeight / 2);

            setfillcolor(RGB(70, 70, 70));
            setlinecolor(RGB(200, 200, 200));
            fillrectangle(buttonX, buttonY, buttonX + buttonWidth, buttonY + buttonHeight);

            settextstyle((int)(laneHeight / 2), 0, L"Arial");
            settextcolor(WHITE);
            outtextxy(buttonX + 10, buttonY, i < laneCount / 2 ? L"\u2192" : L"\u2190");
        }

        // 检查鼠标点击
        if (MouseHit())
        {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN)
            {
                if (msg.x < 45)
                {
                    int clickedLane = msg.y / laneHeight;
                    if (clickedLane >= 0 && clickedLane < laneCount)
                    {
                        clearLane(vehicles, clickedLane);
                    }
                }
            }
        }

        // 生成新车
        if (rand() % 10 == 0)
        {
            int lane = rand() % 6;
            int newX = lane < 3 ? 0 : windowWidth;
            int newY = laneHeight * lane + (int)(0.5 * laneHeight);
            bool isPositionSafe = true;
            int carwidth = RandomGenerator{normalwidth}() * scale * bridge.widthScale;
            int carlength = RandomGenerator{normallength}() * scale;

            for (const auto &existingVehicle : vehicles)
            {
                if (existingVehicle->lane != lane)
                    continue;

                int distance = abs(existingVehicle->x - newX) -
                               (existingVehicle->carlength / 2 + carlength / 2);

                if (distance < SAFE_DISTANCE)
                {
                    isPositionSafe = false;
                    break;
                }
            }

            if (isPositionSafe)
            {
                int vehicleType = rand() % 3;
                Vehicle *newVehicle = nullptr;

                if (vehicleType == 0)
                {
                    newVehicle = new Sedan(
                        lane,
                        carlength,
                        carwidth,
                        newX,
                        newY,
                        (int)rng.generate());
                }
                else if (vehicleType == 1)
                {
                    newVehicle = new SUV(
                        lane,
                        carlength,
                        carwidth,
                        newX,
                        newY,
                        (int)rng.generate());
                }
                else
                {
                    newVehicle = new Truck(
                        lane,
                        carlength,
                        carwidth,
                        newX,
                        newY,
                        (int)rng.generate());
                }

                if (newVehicle)
                {
                    vehicles.push_back(newVehicle);
                }
            }
        }

        // 更新车辆的位置
        int middleY = windowHeight / 2;

        for (auto &v : vehicles)
        {
            COLORREF originalColor = v->color;
            if (v->speed == 0)
            {
                v->handleDangerousSituation();
            }

            // 只有在非变道状态下才调用moveForward函数
            if (!v->isChangingLane) {
                v->moveForward(middleY);
            }
            
            v->checkFrontVehicleDistance(vehicles, v->getSafeDistance());

            if (v->isGoing2change)
            {
                if (v->smoothLaneChange(laneHeight, vehicles))
                {
                    v->haschanged = true;
                }
            }

            if (v->isTooClose)
            {
                bool stillTooClose = false;
                for (const auto other : vehicles)
                {
                    if (other == v)
                        continue;
                    if (other->lane != v->lane)
                        continue;

                    bool isMovingRight = (v->lane < 3);
                    bool isFrontVehicle = isMovingRight ? (other->x > v->x) : (other->x < v->x);

                    if (isFrontVehicle)
                    {
                        int distance = abs(other->x - v->x) -
                                       (other->carlength / 2 + v->carlength / 2);
                        if (distance <= SAFE_DISTANCE)
                        {
                            stillTooClose = true;
                            break;
                        }
                    }
                }

                if (!stillTooClose)
                {
                    v->color = v->originalColor;
                    v->isTooClose = false;
                }
            }
        }

        // 移除离开车辆并释放内存
        vehicles.erase(
            remove_if(vehicles.begin(), vehicles.end(),
                      [windowWidth](Vehicle *v)
                      {
                          if (v->x < 0 || v->x > windowWidth)
                          {
                              delete v; // 释放内存
                              return true;
                          }
                          return false;
                      }),
            vehicles.end());
        // 绘制车辆
        for (const auto &v : vehicles)
        {
            v->predictAndDrawTrajectory(laneHeight, windowHeight / 2, 30, vehicles);

            v->draw();
        }

        Sleep(20);
        time += 0.2;
    }

    // 程序结束前释放所有车辆内存
    for (auto v : vehicles)
    {
        delete v;
    }
    vehicles.clear();

    closegraph();
    return 0;
}
