#include "Class.h"
#include "VehicleTypes.h"
#include <random>

// 随机颜色生成器
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> color_dis(0, 255);

// 生成真实车辆颜色的函数
COLORREF generateVehicleColor() {
    // 定义一些常见的车辆颜色
    static const COLORREF commonColors[] = {
        RGB(255, 255, 255), // 白色
        RGB(0, 0, 0),       // 黑色
        RGB(255, 0, 0),     // 红色
        RGB(0, 0, 255),     // 蓝色
        RGB(255, 255, 0),   // 黄色
        RGB(0, 255, 0),     // 绿色
        RGB(255, 165, 0),   // 橙色
        RGB(128, 0, 128),   // 紫色
        RGB(165, 42, 42),   // 棕色
        RGB(192, 192, 192), // 银色
        RGB(128, 128, 128), // 灰色
        RGB(255, 192, 203), // 粉色
        RGB(75, 0, 130),    // 靛蓝色
        RGB(240, 230, 140), // 像牙黄
        RGB(210, 180, 140), // 卡其色
        RGB(255, 182, 193), // 浅粉红
        RGB(176, 224, 230), // 粉蓝色
        RGB(144, 238, 144), // 浅绿色
        RGB(221, 160, 221), // 梅花色
        RGB(255, 105, 180)  // 热粉红
    };

    std::uniform_int_distribution<> index_dis(0, sizeof(commonColors) / sizeof(COLORREF) - 1);
    return commonColors[index_dis(gen)];
}

// 小轿车类构造函数实现
Sedan::Sedan(int lane, int carlength, int carwidth, int x, int y, int speed)
    : Vehicle(lane, carlength, carwidth, x, y, speed) {
    color = generateVehicleColor(); // 设置随机颜色
}

int Sedan::getSafeDistance() const
{
    return SAFE_DISTANCE * 0.8;  // 比标准安全距离短20%
}

// 小轿车绘制函数 - 真实的侧视图（基于图片参考）
void Sedan::draw() const
{
    int left = x - carlength / 2;
    int right = x + carlength / 2;
    int top = y - carwidth / 2;
    int bottom = y + carwidth / 2;

    bool movingRight = (lane < 3);

    if (isBrokenDown)
    {
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
        // === 小轿车真实侧视图（基于图片参考） ===

        // 车身主体（三厢结构：发动机舱、乘员舱、行李舱）
        setfillcolor(color);
        setlinecolor(RGB(30, 30, 30));
        setlinestyle(PS_SOLID, 2);

        // 车身轮廓 - 使用更符合实际的形状，更贴近图片中的流畅线条
        POINT body[8];
        if (movingRight) {
            // 向右行驶时：车头在右
            body[0] = { left, bottom - carwidth / 8 };  // 后保险杠底部
            body[1] = { left + carlength / 8, top + carwidth / 3 };  // 后轮拱上方
            body[2] = { left + carlength / 4, top + carwidth / 8 };  // 车顶后部
            body[3] = { left + carlength * 2 / 3, top + carwidth / 8 };  // 车顶前部
            body[4] = { right - carlength / 10, top + carwidth / 4 };  // 发动机盖
            body[5] = { right - carlength / 20, top + carwidth / 3 };  // 车头顶部
            body[6] = { right, bottom - carwidth / 4 };  // 车头底部
            body[7] = { left, bottom - carwidth / 8 };  // 闭合
        }
        else {
            // 向左行驶时：车头在左
            body[0] = { right, bottom - carwidth / 8 };  // 后保险杠底部
            body[1] = { right - carlength / 8, top + carwidth / 3 };  // 后轮拱上方
            body[2] = { right - carlength / 4, top + carwidth / 8 };  // 车顶后部
            body[3] = { right - carlength * 2 / 3, top + carwidth / 8 };  // 车顶前部
            body[4] = { left + carlength / 10, top + carwidth / 4 };  // 发动机盖
            body[5] = { left + carlength / 20, top + carwidth / 3 };  // 车头顶部
            body[6] = { left, bottom - carwidth / 4 };  // 车头底部
            body[7] = { right, bottom - carwidth / 8 };  // 闭合
        }

        fillpolygon(body, 7);
        polygon(body, 7);

        // 车窗（更真实的挡风玻璃和侧窗形状）
        setfillcolor(RGB(173, 216, 230)); // 浅蓝色车窗
        setlinecolor(RGB(80, 80, 80));
        setlinestyle(PS_SOLID, 1);

        POINT window[8];
        if (movingRight) {
            // 前挡风玻璃
            window[0] = { left + carlength * 2 / 3, top + carwidth / 6 };
            window[1] = { right - carlength / 10, top + carwidth / 4 };
            window[2] = { right - carlength / 20, top + carwidth / 3 };
            // 侧窗
            window[3] = { left + carlength / 3, top + carwidth / 12 };
            window[4] = { left + carlength * 2 / 3, top + carwidth / 12 };
            // 后挡风玻璃
            window[5] = { left + carlength / 4, top + carwidth / 6 };
            window[6] = { left + carlength / 3, top + carwidth / 12 };
            window[7] = { left + carlength / 4, top + carwidth / 6 };
        }
        else {
            // 前挡风玻璃
            window[0] = { right - carlength * 2 / 3, top + carwidth / 6 };
            window[1] = { left + carlength / 10, top + carwidth / 4 };
            window[2] = { left + carlength / 20, top + carwidth / 3 };
            // 侧窗
            window[3] = { right - carlength / 3, top + carwidth / 12 };
            window[4] = { right - carlength * 2 / 3, top + carwidth / 12 };
            // 后挡风玻璃
            window[5] = { right - carlength / 4, top + carwidth / 6 };
            window[6] = { right - carlength / 3, top + carwidth / 12 };
            window[7] = { right - carlength / 4, top + carwidth / 6 };
        }

        fillpolygon(window, 7);

        // 车门分隔线（更自然的位置）
        setlinecolor(RGB(100, 100, 100));
        if (movingRight) {
            line(left + carlength / 3, top + carwidth / 8, left + carlength / 3, bottom - carwidth / 6);
            line(left + carlength * 2 / 3, top + carwidth / 8, left + carlength * 2 / 3, bottom - carwidth / 6);
        }
        else {
            line(right - carlength / 3, top + carwidth / 8, right - carlength / 3, bottom - carwidth / 6);
            line(right - carlength * 2 / 3, top + carwidth / 8, right - carlength * 2 / 3, bottom - carwidth / 6);
        }

        // 车身腰线（更接近真实轿车的装饰线）
        setlinecolor(RGB(120, 120, 120));
        setlinestyle(PS_SOLID, 2);
        if (movingRight) {
            line(left + carlength / 8, top + carwidth / 2, right - carlength / 8, top + carwidth / 2);
        }
        else {
            line(right - carlength / 8, top + carwidth / 2, left + carlength / 8, top + carwidth / 2);
        }

        // 前后车灯（更真实的形状和位置）
        setfillcolor(movingRight ? RGB(255, 255, 200) : RGB(255, 0, 0)); // 前灯
        if (movingRight) {
            fillrectangle(right - carlength / 10, top + carwidth / 4, right - carlength / 30, top + carwidth / 3);
        }
        else {
            fillrectangle(left + carlength / 30, top + carwidth / 4, left + carlength / 10, top + carwidth / 3);
        }

        setfillcolor(movingRight ? RGB(255, 0, 0) : RGB(255, 255, 200)); // 尾灯
        if (movingRight) {
            fillrectangle(left, top + carwidth / 4, left + carlength / 12, top + carwidth / 3);
        }
        else {
            fillrectangle(right - carlength / 12, top + carwidth / 4, right, top + carwidth / 3);
        }

        // 车轮（更接近真实轿车的大小和样式）
        setfillcolor(BLACK);
        setlinecolor(RGB(50, 50, 50));
        int wheelRadius = max(5, carwidth / 5);
        int wheelY = bottom - wheelRadius / 3; // 离地间隙更真实
        int frontWheelX, rearWheelX;

        if (movingRight) {
            frontWheelX = right - carlength / 4;
            rearWheelX = left + carlength / 4;
        }
        else {
            frontWheelX = left + carlength / 4;
            rearWheelX = right - carlength / 4;
        }

        fillcircle(frontWheelX, wheelY, wheelRadius);
        fillcircle(rearWheelX, wheelY, wheelRadius);

        // 轮毂（更真实的样式）
        setfillcolor(RGB(180, 180, 180));
        int hubRadius = max(3, wheelRadius / 2);
        fillcircle(frontWheelX, wheelY, hubRadius);
        fillcircle(rearWheelX, wheelY, hubRadius);

        // 轮毂辐条（增加细节）
        setlinecolor(RGB(100, 100, 100));
        if (movingRight) {
            line(frontWheelX, wheelY - hubRadius, frontWheelX, wheelY + hubRadius);
            line(frontWheelX - hubRadius, wheelY, frontWheelX + hubRadius, wheelY);
            line(frontWheelX - hubRadius / 1.4f, wheelY - hubRadius / 1.4f, frontWheelX + hubRadius / 1.4f, wheelY + hubRadius / 1.4f);
            line(frontWheelX + hubRadius / 1.4f, wheelY - hubRadius / 1.4f, frontWheelX - hubRadius / 1.4f, wheelY + hubRadius / 1.4f);

            line(rearWheelX, wheelY - hubRadius, rearWheelX, wheelY + hubRadius);
            line(rearWheelX - hubRadius, wheelY, rearWheelX + hubRadius, wheelY);
            line(rearWheelX - hubRadius / 1.4f, wheelY - hubRadius / 1.4f, rearWheelX + hubRadius / 1.4f, wheelY + hubRadius / 1.4f);
            line(rearWheelX + hubRadius / 1.4f, wheelY - hubRadius / 1.4f, rearWheelX - hubRadius / 1.4f, wheelY + hubRadius / 1.4f);
        }
        else {
            line(frontWheelX, wheelY - hubRadius, frontWheelX, wheelY + hubRadius);
            line(frontWheelX - hubRadius, wheelY, frontWheelX + hubRadius, wheelY);
            line(frontWheelX - hubRadius / 1.4f, wheelY - hubRadius / 1.4f, frontWheelX + hubRadius / 1.4f, wheelY + hubRadius / 1.4f);
            line(frontWheelX + hubRadius / 1.4f, wheelY - hubRadius / 1.4f, frontWheelX - hubRadius / 1.4f, wheelY + hubRadius / 1.4f);

            line(rearWheelX, wheelY - hubRadius, rearWheelX, wheelY + hubRadius);
            line(rearWheelX - hubRadius, wheelY, rearWheelX + hubRadius, wheelY);
            line(rearWheelX - hubRadius / 1.4f, wheelY - hubRadius / 1.4f, rearWheelX + hubRadius / 1.4f, wheelY + hubRadius / 1.4f);
            line(rearWheelX + hubRadius / 1.4f, wheelY - hubRadius / 1.4f, rearWheelX - hubRadius / 1.4f, wheelY + hubRadius / 1.4f);
        }

        setlinestyle(PS_SOLID, 1);
    }

    // 显示速度
    wchar_t speedText[16];
    swprintf(speedText, 16, L"%d", speed);
    setbkmode(TRANSPARENT);
    settextcolor(WHITE);
    settextstyle(20, 0, L"Arial");
    outtextxy(x - 10, top - 25, speedText);
}


// SUV类实现
SUV::SUV(int lane, int carlength, int carwidth, int x, int y, int speed)
    : Vehicle(lane, carlength, carwidth, x, y, speed) {
    color = generateVehicleColor(); // 设置随机颜色
}

int SUV::getSafeDistance() const
{
    return SAFE_DISTANCE;
}

// SUV绘制函数 - 真实SUV侧视图（基于图片参考，加入橙色装饰条和梯子）
void SUV::draw() const
{
    int left = x - carlength / 2;
    int right = x + carlength / 2;
    int top = y - carwidth / 2;
    int bottom = y + carwidth / 2;

    bool movingRight = (lane < 3);

    if (isBrokenDown)
    {
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
        // === SUV真实侧视图（基于图片参考，加入橙色装饰条） ===

        // 车身主体
        setfillcolor(color);
        setlinecolor(RGB(30, 30, 30));
        setlinestyle(PS_SOLID, 2);

        // SUV车身轮廓 - 更方正，车顶线条平直，离地间隙更高
        POINT body[8];
        if (movingRight) {
            // 向右行驶时：车头在右
            body[0] = { left, bottom - carwidth / 6 };  // 后保险杠底部
            body[1] = { left + carlength / 8, top + carwidth / 8 };  // 后轮拱上方
            body[2] = { left + carlength / 4, top };  // 车顶后部
            body[3] = { left + carlength * 3 / 4, top };  // 车顶前部
            body[4] = { right - carlength / 10, top + carwidth / 6 };  // 发动机盖
            body[5] = { right, top + carwidth / 3 };  // 车头
            body[6] = { right, bottom - carwidth / 8 };  // 车头底部
            body[7] = { left, bottom - carwidth / 6 };  // 闭合
        }
        else {
            // 向左行驶时：车头在左
            body[0] = { right, bottom - carwidth / 6 };  // 后保险杠底部
            body[1] = { right - carlength / 8, top + carwidth / 8 };  // 后轮拱上方
            body[2] = { right - carlength / 4, top };  // 车顶后部
            body[3] = { right - carlength * 3 / 4, top };  // 车顶前部
            body[4] = { left + carlength / 10, top + carwidth / 6 };  // 发动机盖
            body[5] = { left, top + carwidth / 3 };  // 车头
            body[6] = { left, bottom - carwidth / 8 };  // 车头底部
            body[7] = { right, bottom - carwidth / 6 };  // 闭合
        }

        fillpolygon(body, 7);
        polygon(body, 7);

        // SUV车窗（更大面积的前挡风玻璃，更直立的A柱和C柱）
        setfillcolor(RGB(135, 206, 250)); // 天蓝色
        setlinecolor(RGB(80, 80, 80));
        setlinestyle(PS_SOLID, 1);

        POINT window[6];
        if (movingRight) {
            // 前挡风玻璃
            window[0] = { left + carlength * 3 / 4, top + carwidth / 12 };
            window[1] = { right - carlength / 12, top + carwidth / 4 };
            window[2] = { right - carlength / 15, top + carwidth / 3 };
            // 侧窗
            window[3] = { left + carlength / 3, top + carwidth / 12 };
            window[4] = { left + carlength * 3 / 4, top + carwidth / 12 };
            window[5] = { left + carlength / 3, top + carwidth / 12 };
        }
        else {
            // 前挡风玻璃
            window[0] = { right - carlength * 3 / 4, top + carwidth / 12 };
            window[1] = { left + carlength / 12, top + carwidth / 4 };
            window[2] = { left + carlength / 15, top + carwidth / 3 };
            // 侧窗
            window[3] = { right - carlength / 3, top + carwidth / 12 };
            window[4] = { right - carlength * 3 / 4, top + carwidth / 12 };
            window[5] = { right - carlength / 3, top + carwidth / 12 };
        }

        fillpolygon(window, 6);

        // SUV车门分隔线（通常有4个门，更直立的设计）
        setlinecolor(RGB(100, 100, 100));
        if (movingRight) {
            line(left + carlength / 3, top + carwidth / 8, left + carlength / 3, bottom - carwidth / 6);
            line(left + carlength * 2 / 3, top + carwidth / 8, left + carlength * 2 / 3, bottom - carwidth / 6);
        }
        else {
            line(right - carlength / 3, top + carwidth / 8, right - carlength / 3, bottom - carwidth / 6);
            line(right - carlength * 2 / 3, top + carwidth / 8, right - carlength * 2 / 3, bottom - carwidth / 6);
        }

        // 橙色装饰条（基于图片参考）
        setfillcolor(RGB(255, 165, 0)); // 橙色
        setlinecolor(RGB(255, 165, 0));
        if (movingRight) {
            fillrectangle(left, bottom - carwidth / 4, right, bottom - carwidth / 5);
        }
        else {
            fillrectangle(right, bottom - carwidth / 4, left, bottom - carwidth / 5);
        }

        // 车身下部装饰条
        setlinecolor(RGB(60, 60, 60));
        if (movingRight) {
            line(left + carlength / 8, bottom - carwidth / 8, right - carlength / 8, bottom - carwidth / 8);
        }
        else {
            line(right - carlength / 8, bottom - carwidth / 8, left + carlength / 8, bottom - carwidth / 8);
        }

        // 前后车灯（SUV车灯通常更大更方正）
        setfillcolor(movingRight ? RGB(255, 255, 200) : RGB(255, 0, 0)); // 前灯
        if (movingRight) {
            fillrectangle(right - carlength / 12, top + carwidth / 5, right, top + carwidth / 3);
        }
        else {
            fillrectangle(left, top + carwidth / 5, left + carlength / 12, top + carwidth / 3);
        }

        setfillcolor(movingRight ? RGB(255, 0, 0) : RGB(255, 255, 200)); // 尾灯
        if (movingRight) {
            fillrectangle(left, top + carwidth / 5, left + carlength / 15, top + carwidth / 3);
        }
        else {
            fillrectangle(right - carlength / 15, top + carwidth / 5, right, top + carwidth / 3);
        }

        // SUV车轮（通常更大更宽，轮毂样式更粗犷）
        setfillcolor(BLACK);
        setlinecolor(RGB(50, 50, 50));
        int wheelRadius = max(5, carwidth / 4); // SUV车轮更大
        int wheelY = bottom - wheelRadius / 3; // 离地间隙更大
        int frontWheelX, rearWheelX;

        if (movingRight) {
            frontWheelX = right - carlength / 5; // 前轮位置更靠前
            rearWheelX = left + carlength / 4;
        }
        else {
            frontWheelX = left + carlength / 5; // 前轮位置更靠前
            rearWheelX = right - carlength / 4;
        }

        fillcircle(frontWheelX, wheelY, wheelRadius);
        fillcircle(rearWheelX, wheelY, wheelRadius);

        // SUV轮毂（更大更粗犷）
        setfillcolor(RGB(180, 180, 180));
        int hubRadius = max(3, wheelRadius / 2);
        fillcircle(frontWheelX, wheelY, hubRadius);
        fillcircle(rearWheelX, wheelY, hubRadius);

        // 在SUV侧面绘制梯子（基于图片参考）
        if (movingRight) {
            // 梯子竖杆
            setlinecolor(RGB(100, 100, 100));
            int ladderTop = top + carwidth / 3;
            int ladderBottom = bottom - carwidth / 3;
            int ladderLeft = left + carlength / 4;
            int ladderRight = left + carlength / 4 + 5;

            line(ladderLeft, ladderTop, ladderLeft, ladderBottom);
            line(ladderRight, ladderTop, ladderRight, ladderBottom);

            // 梯子横档
            int rungCount = 4;
            for (int i = 1; i <= rungCount; i++) {
                int rungY = ladderTop + i * (ladderBottom - ladderTop) / (rungCount + 1);
                line(ladderLeft, rungY, ladderRight, rungY);
            }
        }
        else {
            // 梯子竖杆
            setlinecolor(RGB(100, 100, 100));
            int ladderTop = top + carwidth / 3;
            int ladderBottom = bottom - carwidth / 3;
            int ladderLeft = right - carlength / 4 - 5;
            int ladderRight = right - carlength / 4;

            line(ladderLeft, ladderTop, ladderLeft, ladderBottom);
            line(ladderRight, ladderTop, ladderRight, ladderBottom);

            // 梯子横档
            int rungCount = 4;
            for (int i = 1; i <= rungCount; i++) {
                int rungY = ladderTop + i * (ladderBottom - ladderTop) / (rungCount + 1);
                line(ladderLeft, rungY, ladderRight, rungY);
            }
        }

        setlinestyle(PS_SOLID, 1);
    }

    // 显示速度
    wchar_t speedText[16];
    swprintf(speedText, 16, L"%d", speed);
    setbkmode(TRANSPARENT);
    settextcolor(WHITE);
    settextstyle(20, 0, L"Arial");
    outtextxy(x - 10, top - 25, speedText);
}

// 大卡车类实现
Truck::Truck(int lane, int carlength, int carwidth, int x, int y, int speed)
    : Vehicle(lane, carlength, carwidth, x, y, speed) {
    color = generateVehicleColor(); // 设置随机颜色
}

int Truck::getSafeDistance() const
{
    return SAFE_DISTANCE * 1.5;
}

// 大卡车绘制函数 - 真实的货车侧视图
void Truck::draw() const
{
    int left = x - carlength / 2;
    int right = x + carlength / 2;
    int top = y - carwidth / 2;
    int bottom = y + carwidth / 2;

    bool movingRight = (lane < 3);

    if (isBrokenDown)
    {
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
        // === 真实货车侧视图：驾驶舱 + 货厢 ===

        int cabLength = max(15, carlength / 3);  // 驾驶舱长度
        int cabLeft, cabRight, cargoLeft, cargoRight;

        if (movingRight) {
            cabLeft = right - cabLength;
            cabRight = right;
            cargoLeft = left;
            cargoRight = cabLeft;
        }
        else {
            cabLeft = left;
            cabRight = left + cabLength;
            cargoLeft = cabRight;
            cargoRight = right;
        }

        setlinecolor(RGB(30, 30, 30));
        setlinestyle(PS_SOLID, 2);

        // 1. 货厢（全封闭，没有窗户）
        setfillcolor(color);
        fillrectangle(cargoLeft, top, cargoRight, bottom);

        // 货厢竖筋
        setlinecolor(RGB(100, 100, 100));
        int ribCount = max(3, abs(cargoRight - cargoLeft) / 25);
        for (int i = 1; i < ribCount; ++i) {
            int ribX = cargoLeft + i * (cargoRight - cargoLeft) / ribCount;
            line(ribX, top, ribX, bottom);
        }

        // 2. 驾驶舱
        setfillcolor(RGB(220, 220, 220));
        setlinecolor(RGB(30, 30, 30));
        fillroundrect(cabLeft, top, cabRight, bottom, 6, 6);

        // 驾驶舱车窗（只占驾驶舱门的上半部分）
        int cabWindowTop = top + carwidth / 3;
        int cabWindowBot = top + carwidth * 2 / 3;
        int cabWindowLeft = cabLeft + cabLength / 6;
        int cabWindowRight = cabRight - cabLength / 6;

        setfillcolor(RGB(135, 206, 250));
        setlinecolor(RGB(80, 80, 80));
        setlinestyle(PS_SOLID, 1);
        fillrectangle(cabWindowLeft, cabWindowTop, cabWindowRight, cabWindowBot);

        // 驾驶舱车门分隔
        setlinecolor(RGB(100, 100, 100));
        int cabDoor = (cabLeft + cabRight) / 2;
        line(cabDoor, cabWindowTop, cabDoor, cabWindowBot);
        line(cabDoor, cabWindowBot, cabDoor, bottom - carwidth / 5);

        // 前灯
        setfillcolor(RGB(255, 255, 200));
        int lightSize = max(3, carwidth / 6);
        int lightY = cabWindowTop;
        if (movingRight) {
            fillrectangle(cabRight - 3, lightY, cabRight, lightY + lightSize);
        }
        else {
            fillrectangle(cabLeft, lightY, cabLeft + 3, lightY + lightSize);
        }

        // 尾灯
        setfillcolor(RGB(255, 0, 0));
        if (movingRight) {
            fillrectangle(cargoLeft, lightY, cargoLeft + 3, lightY + lightSize);
        }
        else {
            fillrectangle(cargoRight - 3, lightY, cargoRight, lightY + lightSize);
        }

        // 车轮（三轴）
        setfillcolor(BLACK);
        setlinecolor(RGB(50, 50, 50));
        int wheelRadius = max(4, carwidth / 4);
        int wheelY = bottom - wheelRadius / 2;

        // 驾驶舱轮
        int cabWheelX = (cabLeft + cabRight) / 2;
        fillcircle(cabWheelX, wheelY, wheelRadius);

        // 货厢双轮
        int cargo1X = cargoLeft + abs(cargoRight - cargoLeft) * 2 / 3;
        int cargo2X = cargoLeft + abs(cargoRight - cargoLeft) * 5 / 6;
        fillcircle(cargo1X, wheelY, wheelRadius);
        fillcircle(cargo2X, wheelY, wheelRadius);

        // 轮毂
        setfillcolor(RGB(180, 180, 180));
        int hubRadius = max(2, wheelRadius / 2);
        fillcircle(cabWheelX, wheelY, hubRadius);
        fillcircle(cargo1X, wheelY, hubRadius);
        fillcircle(cargo2X, wheelY, hubRadius);

        setlinestyle(PS_SOLID, 1);
    }

    // 显示速度
    wchar_t speedText[16];
    swprintf(speedText, 16, L"%d", speed);
    setbkmode(TRANSPARENT);
    settextcolor(WHITE);
    settextstyle(20, 0, L"Arial");
    outtextxy(x - 10, top - 25, speedText);
}