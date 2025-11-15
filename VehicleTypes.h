#include "Class.h"
using namespace std;
// 小轿车类
struct Sedan : public Vehicle
{
    Sedan(int lane, int carlength, int carwidth, int x, int y, int speed);
    // 重写变道函数，实现更快的变道曲线
    float curveFunc(float t) const override { return 4 * t * t * t - 3 * t * t * t * t; } // 先快后慢的变道曲线
    // 获取小轿车的安全距离
    int getSafeDistance() const override;
    // 重写绘制函数
    void draw() const override;
};

// SUV类
struct SUV : public Vehicle
{
    SUV(int lane, int carlength, int carwidth, int x, int y, int speed);
    // 重写变道函数，实现中等的变道曲线
    float curveFunc(float t) const override { return 3 * t * t - 2 * t * t * t; } // 先快后慢的变道曲线
    // 获取SUV的安全距离
    int getSafeDistance() const override;
    // 重写绘制函数
    void draw() const override;
};

// 大卡车类
struct Truck : public Vehicle
{
    Truck(int lane, int carlength, int carwidth, int x, int y, int speed);
    // 重写变道函数，实现更慢的变道曲线
    float curveFunc(float t) const override { return 2*t*t-t*t*t; } // 先快后慢的变道曲线
    // 获取大卡车的安全距离
    int getSafeDistance() const override;
    // 重写绘制函数
    void draw() const override;
};