#pragma once

void drawDashedLine(int x1, int y1, int x2, int y2);

// 基础常量定义
const int SAFE_DISTANCE = 700;      // 安全距离（像素）
const int CRASH_DISTANCE = 25;      // 碰撞距离（像素）
const int WAIT = 10;                // 等待速度差阈值
const int CRASH = 100;               // 危险速度差阈值

// 可调节参数变量（默认值）
extern int vehicleGenerationFrequency; // 车辆生成频率（数值越小生成越频繁，1表示每次循环都尝试生成）
extern int safeDistance;               // 安全距离
extern int stoppingSpeed;              // 停止速度（减速度值，像素/帧）