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
#include "BridgeLightingControl.h"
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
    
    // UI 布局参数
    const int controlBarWidth = 60;    // 右侧竖直控制栏宽度（独立于车道）
    const int topBarHeight = 80;       // 顶部横向参数栏高度（在最上面再开一条 bar）
    const int roadWidth = windowWidth - controlBarWidth; // 道路宽度，不包含右侧控制栏

    // 创建天气效果管理器（限制在 roadWidth 内，避免绘制到右侧控制栏）
    WeatherEffectManager weatherManager(roadWidth, windowHeight);
    
    // 天气按钮尺寸与居中位置（在顶部 bar 的中间）
    const int btnWidth = 80;
    const int btnHeight = 35;
    const int btnSpacing = 10;
    const int totalWeatherWidth = 3 * btnWidth + 2 * btnSpacing;
    const int btnStartX = (roadWidth - totalWeatherWidth) / 2;
    const int btnStartY = (topBarHeight - btnHeight) / 2;
    
    // 三个按钮的矩形区域
    struct ButtonRect {
        int x1, y1, x2, y2;
        WeatherMode mode;
        const wchar_t* text;
    };
    
    ButtonRect weatherButtons[3] = {
        {btnStartX, btnStartY, btnStartX + btnWidth, btnStartY + btnHeight, NOTHING, L"正常"},
        {btnStartX + btnWidth + btnSpacing, btnStartY, btnStartX + 2 * btnWidth + btnSpacing, btnStartY + btnHeight, RAIN, L"下雨"},
        {btnStartX + 2 * (btnWidth + btnSpacing), btnStartY, btnStartX + 3 * btnWidth + 2 * btnSpacing, btnStartY + btnHeight, SNOW, L"下雪"}
    };
    
    // 绘制 UI 的 lambda（只绘制 top bar + right control bar + 按钮）
    auto drawUI = [&](WeatherEffectManager &wm) {
        // 顶部横条（覆盖全宽，修复右上角黑色小矩形）
        setfillcolor(RGB(40, 40, 40));
        setlinecolor(RGB(80, 80, 80));
        fillrectangle(0, 0, windowWidth, topBarHeight);
        rectangle(0, 0, windowWidth, topBarHeight);

        // 在右侧绘制独立的竖直控制栏背景（从 topBarHeight 开始）
        setfillcolor(RGB(50, 50, 50));
        setlinecolor(RGB(100, 100, 100));
        fillrectangle(roadWidth, topBarHeight, windowWidth, windowHeight);
        rectangle(roadWidth, topBarHeight, windowWidth, windowHeight);

        // 显示桥的参数信息（放在顶部 bar 左侧）
        wchar_t info[256];
        swprintf_s(info, L"桥长： %.0fm  桥宽：%.0fm  桥宽放大率： %.1f",
                   bridge.bridgeLength, bridge.bridgeWidth, bridge.widthScale);
        settextstyle(20, 0, L"Arial");
        settextcolor(WHITE);
        setbkmode(TRANSPARENT);
        outtextxy(10, (topBarHeight - 20) / 2, info);
        
        // 显示时间（放在顶部 bar 右侧，但仍在 roadWidth 范围内）
        wchar_t info2[256];
        swprintf_s(info2, L"时间： %.0fs", time);
        settextstyle(20, 0, L"Arial");
        outtextxy(roadWidth - 160, (topBarHeight - 20) / 2, info2);
        
        // 显示当前天气状态（放在天气按钮上方，居中）
        wchar_t weatherInfo[128];
        WeatherMode currentWeather = wm.getCurrentWeather();
        const wchar_t* weatherText = L"正常";
        if (currentWeather == RAIN) weatherText = L"下雨";
        else if (currentWeather == SNOW) weatherText = L"下雪";
        swprintf_s(weatherInfo, L"当前天气： %s", weatherText);
        settextstyle(18, 0, L"Arial");
        int weatherInfoW = textwidth(weatherInfo);
        outtextxy((roadWidth - weatherInfoW) / 2, btnStartY - 24, weatherInfo);

        // 绘制天气控制按钮（在顶部横条中间）
        for (int i = 0; i < 3; i++)
        {
            bool isActive = (weatherButtons[i].mode == currentWeather);
            
            if (isActive)
            {
                setfillcolor(RGB(0, 120, 215));  // 蓝色高亮
                setlinecolor(RGB(0, 84, 153));
            }
            else
            {
                setfillcolor(RGB(70, 70, 70));   // 灰色
                setlinecolor(RGB(200, 200, 200));
            }
            
            fillroundrect(weatherButtons[i].x1, weatherButtons[i].y1, 
                         weatherButtons[i].x2, weatherButtons[i].y2, 8, 8);
            
            settextstyle(22, 0, L"Arial");
            settextcolor(WHITE);
            setbkmode(TRANSPARENT);
            
            int textX = weatherButtons[i].x1 + (btnWidth - textwidth(weatherButtons[i].text)) / 2;
            int textY = weatherButtons[i].y1 + (btnHeight - textheight(weatherButtons[i].text)) / 2;
            outtextxy(textX, textY, weatherButtons[i].text);
        }

        // 在右侧竖直控制栏绘制每车道的清除抛锚按钮（竖直排列，独立于车道）
        int laneCount = 6;
        int laneHeight = (int)((windowHeight - topBarHeight) / laneCount);
        for (int i = 0; i < laneCount; ++i)
        {
            int buttonWidth = 40;
            int buttonHeight = (int)(laneHeight / 2);
            int buttonX = roadWidth + (controlBarWidth - buttonWidth) / 2; // 居中于右侧控制栏
            int buttonY = topBarHeight + laneHeight * i + (int)(0.5 * laneHeight) - (int)(buttonHeight / 2);

            setfillcolor(RGB(70, 70, 70));
            setlinecolor(RGB(200, 200, 200));
            fillroundrect(buttonX, buttonY, buttonX + buttonWidth, buttonY + buttonHeight, 8, 8);

            settextstyle((int)(laneHeight / 2), 0, L"Arial");
            settextcolor(WHITE);
            setbkmode(TRANSPARENT);
            outtextxy(buttonX + 10, buttonY, i < laneCount / 2 ? L"\u2192" : L"\u2190");
        }
    };

    // 首次绘制 UI
    drawUI(weatherManager);

    bool running = true;
    while (running)
    {
        // 只清除并重绘“道路区域”（不清空 UI bar），以避免按钮响应问题
        setfillcolor(BLACK);
        fillrectangle(0, topBarHeight, roadWidth, windowHeight);

        // 绘制天气效果（限制在 roadWidth 内，已在 WeatherEffectManager 初始化时指定）
        weatherManager.update();
        
        // 绘制车道（车道区域从 y=topBarHeight 到 windowHeight，高度按 laneCount 分配）
        setlinecolor(WHITE);
        settextcolor(WHITE);
        int laneCount = 6;
        int laneHeight = (int)((windowHeight - topBarHeight) / laneCount);
        for (int i = 0; i < laneCount - 1; ++i)
        {
            drawDashedLine(0, topBarHeight + (i + 1) * laneHeight, roadWidth, topBarHeight + (i + 1) * laneHeight);
        }
        
        // 检查鼠标点击（UI 点击事件需要立即反馈 -> 点击时重绘 UI）
        bool uiNeedsRedraw = false;
        if (MouseHit())
        {
            MOUSEMSG msg = GetMouseMsg();
            if (msg.uMsg == WM_LBUTTONDOWN)
            {
                // 检查是否点击右侧控制栏中的车道清除按钮
                if (msg.x >= roadWidth && msg.x <= windowWidth)
                {
                    int clickedLane = (msg.y - topBarHeight) / laneHeight;
                    if (clickedLane >= 0 && clickedLane < laneCount)
                    {
                        clearLane(vehicles, clickedLane);
                        uiNeedsRedraw = true; // 立即重绘 control bar，这样按钮状态/视觉立即响应
                    }
                }
                
                // 检查是否点击天气按钮（位于顶部横条中间）
                for (int i = 0; i < 3; i++)
                {
                    if (msg.x >= weatherButtons[i].x1 && msg.x <= weatherButtons[i].x2 &&
                        msg.y >= weatherButtons[i].y1 && msg.y <= weatherButtons[i].y2)
                    {
                        weatherManager.setWeather(weatherButtons[i].mode);
                        uiNeedsRedraw = true; // 立即重绘 top bar 上的天气按钮显示
                        break;
                    }
                }
            }
        }

        // 如果需要，立即重绘 UI（只重绘 top bar 和右控制栏，不清空 road 区域）
        if (uiNeedsRedraw) {
            drawUI(weatherManager);
        }
        
        // 保留键盘快捷键（可选）
        if (_kbhit())
        {
            char key = _getch();
            if (key == 'r' || key == 'R') {
                weatherManager.setWeather(RAIN);
                drawUI(weatherManager);
            }
            else if (key == 's' || key == 'S') {
                weatherManager.setWeather(SNOW);
                drawUI(weatherManager);
            }
            else if (key == 'n' || key == 'N') {
                weatherManager.setWeather(NOTHING);
                drawUI(weatherManager);
            }
            else if (key == 27 || key == 'q' || key == 'Q') {
                running = false;
            }
        }

        // 生成新车（生成 X 坐标限制在 roadWidth 范围内，不会在控制栏生成）
        if (rand() % 10 == 0)
        {
            int lane = rand() % 6;
            int carwidth = RandomGenerator{normalwidth}() * scale * bridge.widthScale;
            int carlength = RandomGenerator{normallength}() * scale;
            // 起始 X：向右行驶的车辆从左侧进入（左侧可从 -carlength/2 开始）
            // 向左行驶的车辆从 roadWidth 内侧开始，避免出现在 controlBar 上
            int newX = (lane < 3) ? (-carlength / 2 - 5) : (roadWidth - carlength / 2 - 1);
            int newY = topBarHeight + laneHeight * lane + (int)(0.5 * laneHeight);
            bool isPositionSafe = true;

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
        int middleY = (windowHeight + topBarHeight) / 2;

        // 收集需要立即移除的车辆（避免车辆进入 control bar）
        vector<Vehicle*> removeNow;

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

            // 绝对禁止车辆进入 control bar 区域：
            int halfLen = v->carlength / 2;
            if (v->lane < 3) {
                // 向右行驶：一旦车头触及道路右边界，则将该车标记为离开（删除），避免进入 controlBar
                if (v->x + halfLen >= roadWidth - 1) {
                    removeNow.push_back(v);
                    continue;
                }
            } else {
                // 向左行驶：正常从道路左端离开即可（若车尾越过左侧界外则删除）
                if (v->x + halfLen < 0) {
                    removeNow.push_back(v);
                    continue;
                }
            }

            // 预测与绘制轨迹和车辆（仅对未被标记删除的车辆）
            v->predictAndDrawTrajectory(laneHeight, (windowHeight + topBarHeight) / 2, 30, vehicles);
            v->draw();
        }

        // 删除被标记的车辆（它们不会进入 control bar）
        if (!removeNow.empty()) {
            for (auto rem : removeNow) {
                // 从 vehicles 中移除并 delete
                auto it = find(vehicles.begin(), vehicles.end(), rem);
                if (it != vehicles.end()) {
                    delete *it;
                    vehicles.erase(it);
                }
            }
        }

        // 备份删除逻辑：如果有意外的车辆坐标超出左右极限，仍进行清理
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

        Sleep(40);
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
