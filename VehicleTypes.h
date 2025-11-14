#include "Class.h"
#include <vector>       // 因为你使用了 std::vector
#include <functional>   // 因为你使用了 std::function
using namespace std;
// 小轿车类
struct Sedan : public Vehicle {
    Sedan(int lane, int carlength, int carwidth, int x, int y, int speed);
    // 获取小轿车的安全距离
    int getSafeDistance() const override;
    // 重写绘制函数
    void draw() const override;

};

// SUV类
struct SUV : public Vehicle {
    SUV(int lane, int carlength, int carwidth, int x, int y, int speed);
    // 获取SUV的安全距离
    int getSafeDistance() const override;
    // 重写绘制函数
    void draw() const override;
};

// 大卡车类
struct Truck : public Vehicle {
    Truck(int lane, int carlength, int carwidth, int x, int y, int speed);
    // 获取大卡车的安全距离
    int getSafeDistance() const override;
    // 重写绘制函数
    void draw() const override;
};
