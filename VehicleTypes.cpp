#include "Class.h"
#include "VehicleTypes.h"
// 小轿车类构造函数实现
Sedan::Sedan(int lane, int carlength, int carwidth, int x, int y, int speed)
    : Vehicle(lane, carlength, carwidth, x, y, speed) {}

bool Sedan::smoothLaneChange(int laneHeight, const std::vector<Vehicle*> &allVehicles)
{
    // 实现更快的变道曲线
    // 可根据需要自定义变道逻辑
    changeProgress += 0.08f; // 小轿车变道更快
    if (changeProgress >= 1.0f)
    {
        y = endY;
        isChangingLane = false;
        return true;
    }
    y = static_cast<int>(startY + (endY - startY) * changeProgress);
    return false;
}

int Sedan::getSafeDistance() const
{
    // 小轿车安全距离
    return SAFE_DISTANCE * 0.8;  // 比标准安全距离短20%
}

// 小轿车绘制函数
void Sedan::draw() const
{
    int left = x - carlength / 2;
    int right = x + carlength / 2;
    int top = y - carwidth / 2;
    int bottom = y + carwidth / 2;

    if (isBrokenDown)
    {
        // 抛锚车辆：灰色+红色X
        setfillcolor(RGB(100, 100, 100));
        setlinecolor(RGB(50, 50, 50));
        fillroundrect(left, top, right, bottom, 8, 8);

        setlinecolor(RED);
        setlinestyle(PS_SOLID, 3);
        line(x - carlength / 4, y - carwidth / 4, x + carlength / 4, y + carwidth / 4);
        line(x - carlength / 4, y + carwidth / 4, x + carlength / 4, y - carwidth / 4);
        setlinestyle(PS_SOLID, 1);
    }
    else
    {
        // 小轿车
        // 车身 + 阴影
        setfillcolor(RGB(50, 50, 50));
        fillroundrect(left + 2, top + 2, right + 2, bottom + 2, 6, 6);
        setfillcolor(color);
        setlinecolor(RGB(30, 30, 30));
        setlinestyle(PS_SOLID, 2);
        fillroundrect(left, top, right, bottom, 6, 6);
        // 前后窗
        setfillcolor(RGB(150, 200, 230));
        setlinecolor(RGB(80, 80, 80));
        int wm = carlength / 8, wh = carwidth / 3;
        fillrectangle(right - wm - carlength/6, top + wh, right - wm, bottom - wh);
        fillrectangle(left + wm, top + wh, left + wm + carlength/6, bottom - wh);
        // 车轮
        setfillcolor(BLACK);
        int wr = max(2, carwidth / 5), wo = max(4, carlength / 4);
        fillcircle(right - wo, top, wr);
        fillcircle(left + wo, top, wr);
        fillcircle(right - wo, bottom, wr);
        fillcircle(left + wo, bottom, wr);
        // 轮毂
        setfillcolor(RGB(180, 180, 180));
        int hr = max(1, wr / 2);
        fillcircle(right - wo, top, hr);
        fillcircle(left + wo, top, hr);
        fillcircle(right - wo, bottom, hr);
        fillcircle(left + wo, bottom, hr);
    }

    // 在车辆上方显示速度
    wchar_t speedText[16];
    swprintf(speedText,16, L"%d", speed);
    setbkmode(TRANSPARENT);
    settextcolor(WHITE);
    settextstyle(20, 0, L"Arial");
    outtextxy(x - 10, y - carwidth / 2 - 25, speedText);
}

// SUV类实现
SUV::SUV(int lane, int carlength, int carwidth, int x, int y, int speed)
    : Vehicle(lane, carlength, carwidth, x, y, speed) {}

bool SUV::smoothLaneChange(int laneHeight, const std::vector<Vehicle*> &allVehicles)
{
    // SUV变道速度适中
    changeProgress += 0.05f;
    if (changeProgress >= 1.0f)
    {
        y = endY;
        isChangingLane = false;
        return true;
    }
    y = static_cast<int>(startY + (endY - startY) * changeProgress);
    return false;
}

int SUV::getSafeDistance() const
{
    // SUV安全距离
    return SAFE_DISTANCE;  // 使用标准安全距离
}

// SUV绘制函数
void SUV::draw() const
{
    int left = x - carlength / 2;
    int right = x + carlength / 2;
    int top = y - carwidth / 2;
    int bottom = y + carwidth / 2;

    if (isBrokenDown)
    {
        // 抛锚车辆：灰色+红色X
        setfillcolor(RGB(100, 100, 100));
        setlinecolor(RGB(50, 50, 50));
        fillroundrect(left, top, right, bottom, 8, 8);

        setlinecolor(RED);
        setlinestyle(PS_SOLID, 3);
        line(x - carlength / 4, y - carwidth / 4, x + carlength / 4, y + carwidth / 4);
        line(x - carlength / 4, y + carwidth / 4, x + carlength / 4, y - carwidth / 4);
        setlinestyle(PS_SOLID, 1);
    }
    else
    {
        // SUV
        // 车身（更高更长）
        setfillcolor(color);
        setlinecolor(RGB(30, 30, 30));
        fillroundrect(left, top, right, bottom, 8, 8);
        // 侧窗带
        setfillcolor(RGB(180, 220, 240));
        int bandTop = top + carwidth / 5;
        int bandBot = bottom - carwidth / 5;
        fillrectangle(left + carlength/10, bandTop, right - carlength/10, bandBot);
        // 分隔窗格
        setlinecolor(RGB(120, 120, 120));
        int windows = max(4, carlength / 40);
        for (int i = 1; i < windows; ++i)
        {
            int wx = left + carlength/10 + i * (right - left - carlength/5) / windows;
            line(wx, bandTop, wx, bandBot);
        }
        // 车轮（较大）
        setfillcolor(BLACK);
        int wr = max(3, carwidth / 4);
        int w1x = left + carlength / 5;
        int w2x = right - carlength / 5;
        fillcircle(w1x, bottom, wr);
        fillcircle(w2x, bottom, wr);
        setfillcolor(RGB(180, 180, 180));
        fillcircle(w1x, bottom, wr/2);
        fillcircle(w2x, bottom, wr/2);
    }

    // 在车辆上方显示速度
    wchar_t speedText[16];
    swprintf(speedText,16, L"%d", speed);
    setbkmode(TRANSPARENT);
    settextcolor(WHITE);
    settextstyle(20, 0, L"Arial");
    outtextxy(x - 10, y - carwidth / 2 - 25, speedText);
}

// 大卡车类实现
Truck::Truck(int lane, int carlength, int carwidth, int x, int y, int speed)
    : Vehicle(lane, carlength, carwidth, x, y, speed) {}

bool Truck::smoothLaneChange(int laneHeight, const std::vector<Vehicle*> &allVehicles)
{
    // 卡车变道更慢
    changeProgress += 0.03f;
    if (changeProgress >= 1.0f)
    {
        y = endY;
        isChangingLane = false;
        return true;
    }
    y = static_cast<int>(startY + (endY - startY) * changeProgress);
    return false;
}

int Truck::getSafeDistance() const
{
    // 卡车安全距离
    return SAFE_DISTANCE * 1.5;  // 比标准安全距离长50%
}

// 大卡车绘制函数
void Truck::draw() const
{
    int left = x - carlength / 2;
    int right = x + carlength / 2;
    int top = y - carwidth / 2;
    int bottom = y + carwidth / 2;

    if (isBrokenDown)
    {
        // 抛锚车辆：灰色+红色X
        setfillcolor(RGB(100, 100, 100));
        setlinecolor(RGB(50, 50, 50));
        fillroundrect(left, top, right, bottom, 8, 8);

        setlinecolor(RED);
        setlinestyle(PS_SOLID, 3);
        line(x - carlength / 4, y - carwidth / 4, x + carlength / 4, y + carwidth / 4);
        line(x - carlength / 4, y + carwidth / 4, x + carlength / 4, y - carwidth / 4);
        setlinestyle(PS_SOLID, 1);
    }
    else
    {
        // 大卡车
        // 拖挂 + 车头
        int cabLen = max(10, carlength / 4);
        int trailerLeft = left;
        int trailerRight = right - cabLen;
        // 货厢
        setfillcolor(color);
        setlinecolor(RGB(30, 30, 30));
        fillrectangle(trailerLeft, top, trailerRight, bottom);
        // 车头
        setfillcolor(RGB(200, 200, 200));
        fillroundrect(trailerRight, top, right, bottom, 6, 6);
        // 货厢竖筋
        setlinecolor(RGB(100, 100, 100));
        int ribs = max(3, (trailerRight - trailerLeft) / 30);
        for (int i = 1; i < ribs; ++i)
        {
            int rx = trailerLeft + i * (trailerRight - trailerLeft) / ribs;
            line(rx, top, rx, bottom);
        }
        // 车轮（多轴）
        setfillcolor(BLACK);
        int wr = max(3, carwidth / 4);
        int ax1 = trailerLeft + (trailerRight - trailerLeft) * 2 / 3;
        int ax2 = trailerLeft + (trailerRight - trailerLeft) * 4 / 5;
        fillcircle(ax1, bottom, wr);
        fillcircle(ax2, bottom, wr);
        fillcircle(right - cabLen/2, bottom, wr - 1);
    }

    // 在车辆上方显示速度
    wchar_t speedText[16];
    swprintf(speedText,16, L"%d", speed);
    setbkmode(TRANSPARENT);
    settextcolor(WHITE);
    settextstyle(20, 0, L"Arial");
    outtextxy(x - 10, y - carwidth / 2 - 25, speedText);
}
