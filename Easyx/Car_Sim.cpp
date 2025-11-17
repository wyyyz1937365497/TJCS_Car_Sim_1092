#include <graphics.h>
#include <vector>
#include <ctime>
#include <Windows.h>
#include <algorithm>

#include "Random.h"
#include "Class.h"
#include "VehicleTypes.h"
#include "BridgeLightingControl.h"
#include "Define.h"
#include "VehicleStatistics.h"
using namespace std;

// 车辆生成频率控制变量（数值越小生成越频繁，1表示每次循环都尝试生成）
int vehicleGenerationFrequency = 10;
// 安全距离控制变量
int safeDistance = SAFE_DISTANCE;
// 停止速度控制变量
int stoppingSpeed = 15; // 初始值设为15

// 在主循环之前添加函数声明
int getGlobalSafeDistance()
{
    return safeDistance;
}

int getGlobalStoppingSpeed()
{
    return stoppingSpeed;
}

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
    srand((unsigned int)time(0));
    double time = 0;

    // 初始化车辆统计对象
    VehicleStatistics vehicleStats;

    normal_distribution<> normalwidth(3, 0.1);
    normal_distribution<> normallength(6, 0.1);
    uniform_int_distribution<int> int_dist(20, 120);
    RandomGenerator rng(int_dist);

    // UI 布局参数
    const int controlBarWidth = 60;                      // 右侧竖直控制栏宽度（独立于车道）
    const int topBarHeight = 80;                         // 顶部横向参数栏高度（在最上面再开一条 bar）
    const int roadWidth = windowWidth - controlBarWidth; // 道路宽度，不包含右侧控制栏

    // 车辆生成频率控制按钮参数
    const int freqControlWidth = 120;
    const int freqControlHeight = 30;
    const int freqControlX = roadWidth + (controlBarWidth - freqControlWidth) / 2;

    // 创建天气效果管理器（限制在 roadWidth 内，避免绘制到右侧控制栏）
    WeatherEffectManager weatherManager(roadWidth, windowHeight);

    // 当前时间（白天/黑夜）
    TimeOfDay currentTime = TimeOfDay::Day; // 保持默认黑夜显示

    // 天气按钮尺寸与居中位置（在顶部 bar 的中间）
    const int btnWidth = 80;
    const int btnHeight = 35;
    const int btnSpacing = 10;
    const int totalWeatherWidth = 3 * btnWidth + 2 * btnSpacing;
    const int btnStartX = (roadWidth - totalWeatherWidth) / 2;
    const int btnStartY = (topBarHeight - btnHeight) / 2;

    // 增加时间按钮位置（紧挨天气按钮右侧）
    const int timeBtnWidth = 60;
    const int timeBtnHeight = 30;
    const int timeBtnSpacing = 8;
    const int timeBtnStartX = btnStartX + totalWeatherWidth + 12;
    const int timeBtnStartY = (topBarHeight - timeBtnHeight) / 2;

    // 控制按钮参数（频率、安全距离和停止速度控制按钮）
    const int ctrlBtnWidth = 60;
    const int ctrlBtnHeight = 25;
    const int ctrlBtnSpacing = 5;

    // 退出按钮参数
    const int exitBtnWidth = 60;
    const int exitBtnHeight = 30;
    const int exitBtnX = windowWidth - exitBtnWidth - 10;    // 距离右边10像素
    const int exitBtnY = (topBarHeight - exitBtnHeight) / 2; // 垂直居中

    // 频率控制按钮位置
    const int freqCtrlStartX = 10;
    const int freqCtrlStartY = 10;

    // 安全距离控制按钮位置
    const int distCtrlStartX = freqCtrlStartX + 2 * ctrlBtnWidth + ctrlBtnSpacing + 20;
    const int distCtrlStartY = 10;

    // 停止速度控制按钮位置
    const int stopSpeedCtrlStartX = distCtrlStartX + 2 * ctrlBtnWidth + ctrlBtnSpacing + 20;
    const int stopSpeedCtrlStartY = 10;

    // 三个天气按钮的矩形区域
    struct ButtonRect
    {
        int x1, y1, x2, y2;
        WeatherMode mode;
        const wchar_t *text;
    };

    ButtonRect weatherButtons[3] = {
        {btnStartX, btnStartY, btnStartX + btnWidth, btnStartY + btnHeight, NOTHING, L"正常"},
        {btnStartX + btnWidth + btnSpacing, btnStartY, btnStartX + 2 * btnWidth + btnSpacing, btnStartY + btnHeight, RAIN, L"下雨"},
        {btnStartX + 2 * (btnWidth + btnSpacing), btnStartY, btnStartX + 3 * btnWidth + 2 * btnSpacing, btnStartY + btnHeight, SNOW, L"下雪"}};

    // 时间按钮结构
    struct TimeButtonRect
    {
        int x1, y1, x2, y2;
        TimeOfDay time;
        const wchar_t *text;
    };

    TimeButtonRect timeButtons[2] = {
        {timeBtnStartX, timeBtnStartY, timeBtnStartX + timeBtnWidth, timeBtnStartY + timeBtnHeight, TimeOfDay::Day, L"白天"},
        {timeBtnStartX + timeBtnWidth + timeBtnSpacing, timeBtnStartY, timeBtnStartX + 2 * timeBtnWidth + timeBtnSpacing, timeBtnStartY + timeBtnHeight, TimeOfDay::Night, L"黑夜"}};

    // 当前环境照度（lux），用于 UI 显示与 safeDistance 计算
    double currentIlluminance = 0.0;

    // 绘制 UI 的 lambda（只绘制 top bar + right control bar + 按钮）
    auto drawUI = [&](WeatherEffectManager &wm)
    {
        // 顶部横条（覆盖全宽）
        setfillcolor(RGB(40, 40, 40));
        setlinecolor(RGB(80, 80, 80));
        fillrectangle(0, 0, windowWidth, topBarHeight);
        rectangle(0, 0, windowWidth, topBarHeight);

        // 右侧控制栏背景（从 topBarHeight 开始）
        // 绘制退出按钮
        setfillcolor(RGB(180, 70, 70)); // 红色
        setlinecolor(WHITE);
        fillrectangle(exitBtnX, exitBtnY, exitBtnX + exitBtnWidth, exitBtnY + exitBtnHeight);
        rectangle(exitBtnX, exitBtnY, exitBtnX + exitBtnWidth, exitBtnY + exitBtnHeight);

        settextstyle(16, 0, L"Arial");
        settextcolor(WHITE);
        setbkmode(TRANSPARENT);
        const wchar_t *exitText = L"退出";
        int textW = textwidth(exitText);
        int textH = textheight(exitText);
        outtextxy(exitBtnX + (exitBtnWidth - textW) / 2, exitBtnY + (exitBtnHeight - textH) / 2, exitText);

        // 在右侧绘制独立的竖直控制栏背景（从 topBarHeight 开始）
        setfillcolor(RGB(50, 50, 50));
        setlinecolor(RGB(100, 100, 100));
        fillrectangle(roadWidth, topBarHeight, windowWidth, windowHeight);
        rectangle(roadWidth, topBarHeight, windowWidth, windowHeight);

        // 桥的参数信息（顶部 bar 左侧）
        wchar_t info[256];
        swprintf_s(info, L"桥长： %.0fm  桥宽：%.0fm  桥宽放大率： %.1f",
                   bridge.bridgeLength, bridge.bridgeWidth, bridge.widthScale);
        settextstyle(20, 0, L"Arial");
        settextcolor(WHITE);
        setbkmode(TRANSPARENT);
        outtextxy(10, (topBarHeight - 5) / 2, info);

        // 显示时间（放在桥面右上角）
        wchar_t info2[256];
        swprintf_s(info2, L"时间： %.0fs", time);
        settextstyle(20, 0, L"Arial");
        outtextxy(roadWidth - 160, topBarHeight + 10, info2);

        // 显示当前天气状态（放在天气按钮上方，居中）
        wchar_t weatherInfo[128];
        WeatherMode currentWeather = wm.getCurrentWeather();
        const wchar_t *weatherText = L"正常";
        if (currentWeather == RAIN)
            weatherText = L"下雨";
        else if (currentWeather == SNOW)
            weatherText = L"下雪";
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
                setfillcolor(RGB(0, 120, 215)); // 蓝色高亮
                setlinecolor(RGB(0, 84, 153));
            }
            else
            {
                setfillcolor(RGB(70, 70, 70)); // 灰色
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

        // 绘制时间切换按钮（白天/黑夜）
        for (int i = 0; i < 2; ++i)
        {
            bool isActive = (timeButtons[i].time == currentTime);
            if (isActive)
            {
                setfillcolor(RGB(0, 120, 215));
                setlinecolor(RGB(0, 84, 153));
            }
            else
            {
                setfillcolor(RGB(70, 70, 70));
                setlinecolor(RGB(200, 200, 200));
            }
            fillroundrect(timeButtons[i].x1, timeButtons[i].y1, timeButtons[i].x2, timeButtons[i].y2, 6, 6);
            settextstyle(16, 0, L"Arial");
            settextcolor(WHITE);
            int tx = timeButtons[i].x1 + (timeBtnWidth - textwidth(timeButtons[i].text)) / 2;
            int ty = timeButtons[i].y1 + (timeBtnHeight - textheight(timeButtons[i].text)) / 2;
            outtextxy(tx, ty, timeButtons[i].text);
        }

        // 其余 UI 保持不变（频率、安全距离、停止速度按钮等）
        setfillcolor(RGB(70, 70, 180));
        setlinecolor(WHITE);
        fillrectangle(freqCtrlStartX, freqCtrlStartY,
                      freqCtrlStartX + ctrlBtnWidth, freqCtrlStartY + ctrlBtnHeight);
        rectangle(freqCtrlStartX, freqCtrlStartY,
                  freqCtrlStartX + ctrlBtnWidth, freqCtrlStartY + ctrlBtnHeight);

        // 频率减少按钮（修正 rectangle 参数为四个）
        setfillcolor(RGB(70, 180, 70));
        fillrectangle(freqCtrlStartX + ctrlBtnWidth + ctrlBtnSpacing, freqCtrlStartY,
                      freqCtrlStartX + 2 * ctrlBtnWidth + ctrlBtnSpacing, freqCtrlStartY + ctrlBtnHeight);
        rectangle(freqCtrlStartX + ctrlBtnWidth + ctrlBtnSpacing, freqCtrlStartY,
                  freqCtrlStartX + 2 * ctrlBtnWidth + ctrlBtnSpacing, freqCtrlStartY + ctrlBtnHeight);

        setfillcolor(RGB(180, 70, 70));
        fillrectangle(distCtrlStartX, distCtrlStartY,
                      distCtrlStartX + ctrlBtnWidth, distCtrlStartY + ctrlBtnHeight);
        rectangle(distCtrlStartX, distCtrlStartY,
                  distCtrlStartX + ctrlBtnWidth, distCtrlStartY + ctrlBtnHeight);

        // 安全距离减少按钮（修正 rectangle 参数）
        setfillcolor(RGB(180, 180, 70));
        fillrectangle(distCtrlStartX + ctrlBtnWidth + ctrlBtnSpacing, distCtrlStartY,
                      distCtrlStartX + 2 * ctrlBtnWidth + ctrlBtnSpacing, distCtrlStartY + ctrlBtnHeight);
        rectangle(distCtrlStartX + ctrlBtnWidth + ctrlBtnSpacing, distCtrlStartY,
                  distCtrlStartX + 2 * ctrlBtnWidth + ctrlBtnSpacing, distCtrlStartY + ctrlBtnHeight);

        setfillcolor(RGB(70, 180, 180));
        fillrectangle(stopSpeedCtrlStartX, stopSpeedCtrlStartY,
                      stopSpeedCtrlStartX + ctrlBtnWidth, stopSpeedCtrlStartY + ctrlBtnHeight);
        rectangle(stopSpeedCtrlStartX, stopSpeedCtrlStartY,
                  stopSpeedCtrlStartX + ctrlBtnWidth, stopSpeedCtrlStartY + ctrlBtnHeight);

        // 停止速度减少按钮（修正 rectangle 参数）
        setfillcolor(RGB(180, 70, 180));
        fillrectangle(stopSpeedCtrlStartX + ctrlBtnWidth + ctrlBtnSpacing, stopSpeedCtrlStartY,
                      stopSpeedCtrlStartX + 2 * ctrlBtnWidth + ctrlBtnSpacing, stopSpeedCtrlStartY + ctrlBtnHeight);
        rectangle(stopSpeedCtrlStartX + ctrlBtnWidth + ctrlBtnSpacing, stopSpeedCtrlStartY,
                  stopSpeedCtrlStartX + 2 * ctrlBtnWidth + ctrlBtnSpacing, stopSpeedCtrlStartY + ctrlBtnHeight);

        settextstyle(14, 0, L"Arial");
        settextcolor(WHITE);
        setbkmode(TRANSPARENT);

        outtextxy(freqCtrlStartX + 5, freqCtrlStartY + 5, L"频+");
        outtextxy(freqCtrlStartX + ctrlBtnWidth + ctrlBtnSpacing + 5, freqCtrlStartY + 5, L"频-");

        outtextxy(distCtrlStartX + 5, distCtrlStartY + 5, L"距+");
        outtextxy(distCtrlStartX + ctrlBtnWidth + ctrlBtnSpacing + 5, distCtrlStartY + 5, L"距-");

        outtextxy(stopSpeedCtrlStartX + 5, stopSpeedCtrlStartY + 5, L"速+");
        outtextxy(stopSpeedCtrlStartX + ctrlBtnWidth + ctrlBtnSpacing + 5, stopSpeedCtrlStartY + 5, L"速-");

        wchar_t statusText[256];
        swprintf_s(statusText, L"生成频率:%d 探测距离:%d 减速度:%d 照度: %.2f lux", vehicleGenerationFrequency, safeDistance, stoppingSpeed, currentIlluminance);
        outtextxy(stopSpeedCtrlStartX + 2 * ctrlBtnWidth + ctrlBtnSpacing + 10, stopSpeedCtrlStartY + 5, statusText);
        // 显示流明（照度）在顶部 bar（靠近时间左侧）

        // 右侧抛锚清除按钮
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

            int pad = max(4, buttonWidth / 8);
            setlinecolor(RGB(220, 30, 30));
            line(buttonX + pad, buttonY + pad, buttonX + buttonWidth - pad, buttonY + buttonHeight - pad);
            line(buttonX + pad, buttonY + buttonHeight - pad, buttonX + buttonWidth - pad, buttonY + pad);
        }
    };

    // Helper: 将 WeatherMode + currentTime 映射到 BridgeLightingControl::WeatherCondition
    auto mapToWeatherCondition = [&](WeatherMode wm, TimeOfDay tod) -> WeatherCondition
    {
        if (wm == RAIN)
        {
            return (tod == TimeOfDay::Day) ? WeatherCondition::Rain : WeatherCondition::RainNight;
        }
        else if (wm == SNOW)
        {
            return (tod == TimeOfDay::Day) ? WeatherCondition::Snow : WeatherCondition::SnowNight;
        }
        else
        { // NOTHING
            return (tod == TimeOfDay::Day) ? WeatherCondition::Sunny : WeatherCondition::SunnyNight;
        }
    };

    // Helper: 根据天气与时间从 BridgeLightingControl 获取环境配置，并设置 safeDistance 与 stoppingSpeed
    auto applyWeatherToSafety = [&](WeatherMode wm, TimeOfDay tod)
    {
        try
        {
            WeatherCondition cond = mapToWeatherCondition(wm, tod);
            double illuminance_lux = BridgeTrafficController::getEnvironmentConfig(tod, cond);

            // 保存当前照度用于 UI 显示
            currentIlluminance = illuminance_lux;

            // 根据照度调整 safeDistance（照度越强，可视距离越大）
            // 参考标准：
            // 白天晴天 (100000 lux) -> 最大可视距离
            // 夜晚晴天 (10000 lux)  -> 中等可视距离
            // 夜晚雨天 (6000 lux)   -> 较低可视距离
            // 白天雨天 (20000 lux)  -> 中等偏上可视距离
            // 夜晚雪天 (2000 lux)   -> 很低可视距离
            // 白天雪天 (60000 lux)  -> 中等偏上可视距离

            double scaledMeters;
            if (illuminance_lux >= 100000.0)
            {
                // 白天晴天 - 最佳能见度
                scaledMeters = 1200.0;
            }
            else if (illuminance_lux >= 60000.0)
            {
                // 白天雪天 - 中等能见度
                scaledMeters = 400.0;
            }
            else if (illuminance_lux >= 20000.0)
            {
                // 白天雨天 - 良好能见度
                scaledMeters = 800.0;
            }
            else if (illuminance_lux >= 10000.0)
            {
                // 夜晚晴天 - 中等偏低能见度
                scaledMeters = 200.0;
            }
            else if (illuminance_lux >= 6000.0)
            {
                // 夜晚雨天 - 低能见度
                scaledMeters = 100.0;
            }
            else
            {
                // 夜晚雪天 - 极低能见度
                scaledMeters = 50.0;
            }

            safeDistance = scaledMeters;

            // stoppingSpeed 仅受天气影响（白天/夜晚不影响减速度）
            const int baseStopping = 15;
            if (wm == NOTHING)
            {
                stoppingSpeed = baseStopping;
            }
            else if (wm == RAIN)
            {
                stoppingSpeed = max(1, (int)(baseStopping * 0.8));
            }
            else
            { // SNOW
                stoppingSpeed = max(1, (int)(baseStopping * 0.6));
            }
        }
        catch (...)
        {
            // 若获取环境配置失败，不改变现有值
        }

        // 刷新顶部 UI 显示（含照度与参数）
        drawUI(weatherManager);
    };

    // 首次根据当前天气和时间设置安全参数并绘制 UI
    applyWeatherToSafety(weatherManager.getCurrentWeather(), currentTime);
    drawUI(weatherManager);

    bool running = true;
    while (running)
    {
        // 只清除并重绘“道路区域”（不清空 UI bar），以避免按钮响应问题
        if (currentTime == TimeOfDay::Day)
        {
            setfillcolor(RGB(100, 100, 100)); // 柏油路浅灰
        }
        else
        {
            setfillcolor(BLACK);
        }
        fillrectangle(0, topBarHeight, roadWidth, windowHeight);

        // 绘制天气效果（限制在 roadWidth 内）
        weatherManager.update();

        // 绘制车道
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
                // 检查是否点击退出按钮
                if (msg.x >= exitBtnX && msg.x <= exitBtnX + exitBtnWidth &&
                    msg.y >= exitBtnY && msg.y <= exitBtnY + exitBtnHeight)
                {
                    running = false;
                }

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
                        applyWeatherToSafety(weatherButtons[i].mode, currentTime);
                        uiNeedsRedraw = true;
                        break;
                    }
                }

                // 时间按钮（白天/黑夜）
                for (int i = 0; i < 2; ++i)
                {
                    if (msg.x >= timeButtons[i].x1 && msg.x <= timeButtons[i].x2 &&
                        msg.y >= timeButtons[i].y1 && msg.y <= timeButtons[i].y2)
                    {
                        currentTime = timeButtons[i].time;
                        applyWeatherToSafety(weatherManager.getCurrentWeather(), currentTime);
                        uiNeedsRedraw = true;
                        break;
                    }
                }

                // 其余控制按钮（频率、安全距离手动、停止速度手动）
                if (msg.x >= freqCtrlStartX && msg.x <= freqCtrlStartX + ctrlBtnWidth &&
                    msg.y >= freqCtrlStartY && msg.y <= freqCtrlStartY + ctrlBtnHeight)
                {
                    vehicleGenerationFrequency = min(vehicleGenerationFrequency + 1, 100);
                    uiNeedsRedraw = true;
                }
                else if (msg.x >= freqCtrlStartX + ctrlBtnWidth + ctrlBtnSpacing &&
                         msg.x <= freqCtrlStartX + 2 * ctrlBtnWidth + ctrlBtnSpacing &&
                         msg.y >= freqCtrlStartY && msg.y <= freqCtrlStartY + ctrlBtnHeight)
                {
                    vehicleGenerationFrequency = max(vehicleGenerationFrequency - 1, 1); // 最小为1
                    uiNeedsRedraw = true;
                }
                // 安全距离增加按钮
                else if (msg.x >= distCtrlStartX && msg.x <= distCtrlStartX + ctrlBtnWidth &&
                         msg.y >= distCtrlStartY && msg.y <= distCtrlStartY + ctrlBtnHeight)
                {
                    safeDistance = min(safeDistance + 50, 2000); // 最大为2000
                    uiNeedsRedraw = true;
                }
                // 安全距离减少按钮
                else if (msg.x >= distCtrlStartX + ctrlBtnWidth + ctrlBtnSpacing &&
                         msg.x <= distCtrlStartX + 2 * ctrlBtnWidth + ctrlBtnSpacing &&
                         msg.y >= distCtrlStartY && msg.y <= distCtrlStartY + ctrlBtnHeight)
                {
                    safeDistance = max(safeDistance - 50, 100); // 最小为100
                    uiNeedsRedraw = true;
                }
                // 停止速度增加按钮
                else if (msg.x >= stopSpeedCtrlStartX && msg.x <= stopSpeedCtrlStartX + ctrlBtnWidth &&
                         msg.y >= stopSpeedCtrlStartY && msg.y <= stopSpeedCtrlStartY + ctrlBtnHeight)
                {
                    stoppingSpeed = min(stoppingSpeed + 1, 50); // 最大为50
                    uiNeedsRedraw = true;
                }
                // 停止速度减少按钮
                else if (msg.x >= stopSpeedCtrlStartX + ctrlBtnWidth + ctrlBtnSpacing &&
                         msg.x <= stopSpeedCtrlStartX + 2 * ctrlBtnWidth + ctrlBtnSpacing &&
                         msg.y >= stopSpeedCtrlStartY && msg.y <= stopSpeedCtrlStartY + ctrlBtnHeight)
                {
                    stoppingSpeed = max(stoppingSpeed - 1, 1); // 最小为1
                    uiNeedsRedraw = true;
                }
            }
        }
        // 如果需要，立即重绘 UI（只重绘 top bar 和右控制栏，不清空 road 区域）
        if (uiNeedsRedraw)
        {
            drawUI(weatherManager);
        }

        // 检查参数是否改变并记录统计数据
        vehicleStats.checkAndRecordParameters(time, safeDistance, stoppingSpeed, vehicles);

        // 生成新车（生成 X 坐标限制在 roadWidth 范围内，不会在控制栏生成）
        // 根据vehicleGenerationFrequency值控制生成频率
        if (rand() % vehicleGenerationFrequency == 0)
        {
            int lane = rand() % 6;
            int carwidth = RandomGenerator{normalwidth}() * scale * bridge.widthScale;
            int carlength = RandomGenerator{normallength}() * scale;
            // 起始 X：向右行驶的车辆从左侧进入（左侧可从 -carlength/2 开始）
            // 向左行驶的车辆从 roadWidth 内侧开始，避免出现在 controlBar 上
            int newX = (lane < 3) ? (-carlength / 2 - 5) : (roadWidth - carlength / 2 - 1);
            int newY = topBarHeight + laneHeight * lane + (int)(0.5 * laneHeight);
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
                // 记录生成的车辆类型
                vehicleStats.recordVehicle(newVehicle);
            }
        }

        // 更新车辆的位置
        int middleY = (windowHeight + topBarHeight) / 2;

        // 收集需要立即移除的车辆（避免车辆进入 control bar）
        vector<Vehicle *> removeNow;

        for (auto &v : vehicles)
        {
            COLORREF originalColor = v->color;
            v->checkFrontVehicleDistance(vehicles, safeDistance, laneHeight);
            if ((*v).speed < 0)
            {
                (*v).speed = 20;
            }
            if (v->speed == 0)
            {
                v->handleDangerousSituation();
            }

            // 只有在非变道状态下才调用moveForward函数
            if (!v->isChangingLane)
            {
                v->moveForward(middleY);
            }

            if (v->isGoing2change)
            {
                v->smoothLaneChange(laneHeight, vehicles);
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

            int halfLen = v->carlength / 2;
            if (v->lane < 3)
            {
                if (v->x + halfLen >= roadWidth - 1)
                {
                    removeNow.push_back(v);
                    continue;
                }
            }
            else
            {
                // 向左行驶：正常从道路左端离开即可（若车尾越过左侧界外则删除）
                if (v->x + halfLen < 0)
                {
                    removeNow.push_back(v);
                    continue;
                }
            }

            // 在屏幕最左侧绘制车道方向箭头（考虑到顶部横条）
            settextcolor(WHITE);
            setbkmode(TRANSPARENT);
            for (int i = 0; i < laneCount; ++i)
            {
                settextstyle((int)(laneHeight / 2), 0, L"Arial");
                int arrowY = topBarHeight + laneHeight * i + (int)(0.5 * laneHeight) - (int)(laneHeight / 4);
                outtextxy(5, arrowY, i < laneCount / 2 ? L"→" : L"←");
            }
            // 显示时间（放在桥面右上角）
            wchar_t info2[256];
            swprintf_s(info2, L"时间： %.0fs", time);
            settextstyle(20, 0, L"Arial");
            outtextxy(roadWidth - 160, topBarHeight + 10, info2);

            v->predictAndDrawTrajectory(laneHeight, (windowHeight + topBarHeight) / 2, 30, vehicles);
            v->draw();
        }

        // 删除被标记的车辆（它们不会进入 control bar）
        if (!removeNow.empty())
        {
            for (auto rem : removeNow)
            {
                auto it = find(vehicles.begin(), vehicles.end(), rem);
                if (it != vehicles.end())
                {
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
    vehicleStats.saveAllStatistics();
    closegraph();
    return 0;
}

// 在文件末尾添加getSafeDistance函数的实现
int Vehicle::getSafeDistance() const
{
    return getGlobalSafeDistance();
}
