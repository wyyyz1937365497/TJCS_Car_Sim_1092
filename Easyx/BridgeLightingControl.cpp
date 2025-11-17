#include "BridgeLightingControl.h"
#include <map>
#include <stdexcept>
#include <sstream>
#include <graphics.h>
#include <cstdlib>

double BridgeTrafficController::getEnvironmentConfig(TimeOfDay time, WeatherCondition weather)
{
    std::map<WeatherCondition, double> luxMap;
    if (time == TimeOfDay::Day)
    {
        luxMap = {
            {WeatherCondition::Sunny, 100000.0},
            {WeatherCondition::Snow, 60000},
            {WeatherCondition::Rain, 20000}};
    }
    else
    {
        luxMap = {
            {WeatherCondition::SunnyNight,10000},
            {WeatherCondition::RainNight, 6000},
            {WeatherCondition::SnowNight, 2000},
        };
    }

    auto it = luxMap.find(weather);
    if (it == luxMap.end())
    {
        throw std::invalid_argument("Unsupported weather/time combination");
    }

    double lux = it->second;
    return lux;
}

// 天气效果管理器实现
WeatherEffectManager::WeatherEffectManager(int winWidth, int winHeight)
    : windowWidth(winWidth), windowHeight(winHeight), currentWeather(NOTHING)
{
    // 动态分配数组
    rain = new Particle[RAIN_COUNT];
    snow = new Particle[SNOW_COUNT];

    // 初始化雨滴位置（从顶部菜单栏下方开始）
    for (int i = 0; i < RAIN_COUNT; ++i)
    {
        rain[i].x = rand() % windowWidth;
        rain[i].y = 80 + (rand() % (windowHeight - 80)); // 避开顶部80像素的菜单栏
    }
    // 初始化雪花位置（从顶部菜单栏下方开始）
    for (int i = 0; i < SNOW_COUNT; ++i)
    {
        snow[i].x = rand() % windowWidth;
        snow[i].y = 80 + (rand() % (windowHeight - 80)); // 避开顶部80像素的菜单栏
    }
}

WeatherEffectManager::~WeatherEffectManager()
{
    delete[] rain;
    delete[] snow;
}

void WeatherEffectManager::setWeather(WeatherMode mode)
{
    currentWeather = mode;
}

void WeatherEffectManager::update()
{
    if (currentWeather == RAIN)
    {
        // 绘制雨滴
        setlinecolor(RGB(135, 206, 250));
        for (int i = 0; i < RAIN_COUNT; ++i)
        {
            rain[i].y += 10;
            if (rain[i].y > windowHeight)
            {
                rain[i].y = 80; // 从顶部菜单栏下方重新开始
                rain[i].x = rand() % windowWidth;
            }
            // 只在道路区域绘制雨滴，避开顶部菜单栏
            if (rain[i].y >= 80)
            {
                line((int)rain[i].x, (int)rain[i].y, (int)(rain[i].x), (int)(rain[i].y + 6));
            }
        }
    }
    else if (currentWeather == SNOW) {
        // 绘制雪花
        setfillcolor(WHITE);
        setlinecolor(WHITE);
        for (int i = 0; i < SNOW_COUNT; ++i) {
            snow[i].y += 1.5;
            snow[i].x += (rand() % 3 - 1);

            if (snow[i].y > windowHeight) {
                snow[i].y = 80; // 从顶部菜单栏下方重新开始
                snow[i].x = rand() % windowWidth;
            }
            
            if (snow[i].y < 85) {
                snow[i].y = 85; // 确保雪花不会进入菜单栏区域
            }

            if (snow[i].x < 0) snow[i].x = 0;
            if (snow[i].x > windowWidth-5) snow[i].x = windowWidth-5;

            // 只在道路区域绘制雪花，避开顶部菜单栏
            if (snow[i].y >= 85) {
                fillcircle((int)snow[i].x, (int)snow[i].y, 2);
            }
        }
    }
}