void drawDashedLine(int x1, int y1, int x2, int y2);
// 定义安全距离和回调函数
const int SAFE_DISTANCE = 150; // 安全距离（像素）
const int CRASH_DISTANCE = 50; // 碰撞距离（像素）
const int WAIT = 30;           // 等待速度差阈值
const int CRASH = 80;          // 危险速度差阈值
int Sedan_curve(int t);
int SUV_curve(int t);
int Truck_curve(int t);
#pragma once
