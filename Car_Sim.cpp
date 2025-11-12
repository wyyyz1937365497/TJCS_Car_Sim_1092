//#pragma comment(lib,"graphics.lib")
#include <graphics.h>
#include <vector>
#include <ctime>
#include <conio.h> // 需要包含此头文件_kbhit()函数需要
#include <Windows.h>
#include <sstream>
#include <string>
#include <iostream>
#include <random>
#include <functional>
using namespace std;

/*
* https://easyx.cn/  下载
* find location:
* C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\VS\include
*/

//随机数生成，传入分布函数，按照分布函数生成随机数
class RandomGenerator {
private:
    mt19937 gen;  // 随机数引擎
    function<double()> generator;  // 生成函数

public:
    // 模板构造函数：接受任何分布类型
    template<typename Distribution>
    RandomGenerator(Distribution& dist) {
        random_device rd;
        gen.seed(rd());  // 初始化引擎

        // 创建生成函数，绑定分布和引擎
        generator = [&]() { return dist(gen); };
    }
    // 生成随机数
    double generate() {
        return generator();
    }
    // 重载函数调用操作符
    double operator()() {
        return generate();
    }
};

//定义车辆的类
struct Vehicle {
    int lane, carlength, carwidth, x, y, speed, changeposition;
    bool haschanged;
    COLORREF color;

    void draw() const {
        setfillcolor(color);          // 设置填充颜色
        setlinecolor(color);          // 让边框也是同色
        fillrectangle(x - carlength / 2, y - carwidth / 2, x + carlength / 2, y + carwidth / 2);
    }
};

//定义桥的类
struct Bridge {
    double bridgeLength, bridgeWidth, widthScale;
    // 采用此函数计算适合屏幕的窗口尺寸和缩放比例，准备绘制桥梁、车道
    // 根据屏幕分辨率调整窗口大小
    void calculateWindowSize(int& windowWidth, int& windowHeight, double& scale) const {
        int margin = 100;      //边缘留白
        //获取屏幕分辨率
        int maxscreenWidth = GetSystemMetrics(SM_CXSCREEN) - margin;
        int maxscreenHeight = GetSystemMetrics(SM_CYSCREEN) - margin;

        windowWidth = (int)bridgeLength;
        windowHeight = (int)(bridgeWidth * widthScale);
        // 确保窗口不超过屏幕分辨率
        double scaleX = static_cast<double>(maxscreenWidth) / windowWidth;
        double scaleY = static_cast<double>(maxscreenHeight) / windowHeight;
        double finalScaleFactor = min(scaleX, scaleY);

        scale = finalScaleFactor;
        windowWidth = int(windowWidth * finalScaleFactor);
        windowHeight = int(windowHeight * finalScaleFactor);

        initgraph(windowWidth, windowHeight);
    }
};

//绘制虚线
void drawDashedLine(int x1, int y1, int x2, int y2) {
    int dashLength = 10; // 虚线段长度
    int gapLength = 10; // 空白段长度
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = max(abs(dx), abs(dy));
    float xIncrement = (float)dx / steps;
    float yIncrement = (float)dy / steps;

    for (int i = 0; i < steps; i += dashLength + gapLength) {
        int startX = round(x1 + i * xIncrement);
        int startY = round(y1 + i * yIncrement);
        int endX = round(startX + dashLength * xIncrement);
        int endY = round(startY + dashLength * yIncrement);

        if (endX > x2 || endY > y2) break; // 防止超出终点

        line(startX, startY, endX, endY); // 绘制虚线段
    }
}

int main() {

    Bridge bridge;
    //输入桥梁参数
    cout << "请输入桥长（m）: ";
    cin >> bridge.bridgeLength;
    cout << "请输入桥宽（m）: ";
    cin >> bridge.bridgeWidth;
    cout << "请输入桥宽放大率: ";
    cin >> bridge.widthScale;
    //调用桥梁绘制函数计算窗口大小
    int windowWidth, windowHeight;
    double scale;
    bridge.calculateWindowSize(windowWidth, windowHeight, scale);

    vector<Vehicle> vehicles;
    srand(unsigned int(time(0)));
    double time = 0;

    //车辆长宽的分布，随机数取值
    normal_distribution<> normalwidth(3, 0.1);  //车的宽度  这里用了正态分布
    normal_distribution<> normallength(6, 0.1); //车辆长度  这里用了正态分布


    while (!_kbhit()) {
        cleardevice();
        // 显示桥的参数信息
        wchar_t info[256];
        swprintf_s(info, L"桥长： %.0fm  桥宽：%.0fm  桥宽放大率： %.1f", bridge.bridgeLength, bridge.bridgeWidth, bridge.widthScale);
        settextstyle(20, 0, L"Arial");
        outtextxy(10, 10, info);
        //显示时间
        wchar_t info2[256];
        swprintf_s(info2, L"时间： %.0fs", time);
        settextstyle(20, 0, L"Arial");
        outtextxy(windowWidth - 150, 10, info2);

        // 绘制车道
        setlinecolor(WHITE);        // 设置线条为白色
        settextcolor(WHITE);        // 设置文字为白色
        int laneCount = 6;//车道数量
        int laneHeight = (int)(windowHeight / laneCount);//车道像素宽度
        for (int i = 0; i < laneCount - 1; ++i) {
            drawDashedLine(0, (i + 1) * laneHeight, windowWidth, (i + 1) * laneHeight);
        }
        //绘制箭头
        for (int i = 0; i < laneCount; ++i) {
            settextstyle((int)(laneHeight / 2), 0, L"Arial");
            outtextxy(5, laneHeight * i + (int)(0.5 * laneHeight) - (int)(laneHeight / 4), i < laneCount / 2 ? L"→" : L"←");
        }


        // 生成新车
        if (rand() % 20 == 0) {  //判断要不要产生新的一辆车
            int lane = rand() % 6;   //如果有车，车辆的随机位置

            //随机的车辆长与宽          
            int carwidth = RandomGenerator{ normalwidth }() * scale * bridge.widthScale;
            int carlength = RandomGenerator{ normallength }() * scale;

            //车辆变道位置  //这里将一个车是否变道，放在了车辆上桥前就决定了
            //此处大家可以优化
            normal_distribution<> normaltime(windowWidth / 2, 0.1);  //随机了变道位置概率分布
            int changeposition = RandomGenerator{ normaltime }();  //随机了变道位置

            vehicles.push_back(Vehicle{  //桥梁上所有车子存放在vehicles里面，
                //添加行车
lane,
carlength,
carwidth,
lane < 3 ? 0 : windowWidth,
laneHeight * lane + (int)(0.5 * laneHeight),
(int)((rand() % 3 + 1) * scale),
changeposition,
false,
RGB(rand() % 256, rand() % 256, rand() % 256),
                });
        }

        // 更新车辆的位置
        int middleY = windowHeight / 2;  //桥面中心的位置
        for (auto& v : vehicles) {
            v.x += (v.y < middleY) ? v.speed : -v.speed;//直线行驶，上行、下行分开
            //变道的逻辑，这里大家看看如何变道
            if (v.haschanged == false) {
                if ((v.y < middleY && v.x >= v.changeposition) ||    //判断要不要变道
                    (v.y > middleY && v.x <= v.changeposition))
                {
                    if (v.lane == 0 || v.lane == 3)  //   只能向相邻位置变道
                    {
                        v.y += laneHeight;
                        v.lane++;
                    }
                    else if (v.lane == 2 || v.lane == 5)
                    {
                        v.y -= laneHeight;
                        v.lane--;
                    }
                    else if (v.lane == 1 || v.lane == 4)   //随机方向变道
                    {
                        int offset = rand() % 2 ? 1 : -1;
                        v.y -= laneHeight * offset;
                        v.lane += offset;
                    }
                    v.haschanged = true;
                }
            }
        }
        // 移除离开车辆
        vehicles.erase(remove_if(vehicles.begin(), vehicles.end(),
            [windowWidth](const Vehicle& v) { return v.x <0 || v.x> windowWidth; }), vehicles.end()); //remove_if:遍历所有车辆，将不需要删除的车辆移至前方，
        //将需要删除的移至后方，返回一个分界点值，erase删除从分界点到末尾的值

// 绘制车辆
        for (const auto& v : vehicles) v.draw();

        Sleep(20);//ms
        time += 0.2;
    }
    closegraph();
    return 0;
}
