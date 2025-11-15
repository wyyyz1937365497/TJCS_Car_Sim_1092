#include "BridgeLightingControl.h"
#include <map>
#include <stdexcept>
#include <sstream>
#include <graphics.h>
#include <cstdlib>

std::string EnvironmentConfig::toString() const {
    std::ostringstream oss;
    oss << "Illuminance: " << illuminance_lux << " lux, "
        << "Speed: " << recommended_speed_kmh << " km/h, "
        << "Distance: " << min_safe_distance_m << " m";
    return oss.str();
}

EnvironmentConfig BridgeTrafficController::getEnvironmentConfig(TimeOfDay time, WeatherCondition weather) {
    std::map<WeatherCondition, double> luxMap;
    if (time == TimeOfDay::Day) {
        luxMap = {
            {WeatherCondition::Sunny, 100000.0},
            {WeatherCondition::Cloudy, 20000.0},
            {WeatherCondition::Overcast, 5000.0},
            {WeatherCondition::LightRain, 2000.0},
            {WeatherCondition::ModerateRain, 1500.0},
            {WeatherCondition::HeavyRain, 500.0}
        };
    }
    else {
        luxMap = {
            {WeatherCondition::ClearNight, 0.2},
            {WeatherCondition::CloudyNight, 0.15},
            {WeatherCondition::RainyNight, 0.03},
            {WeatherCondition::LitBridge, 15.0}
        };
    }

    auto it = luxMap.find(weather);
    if (it == luxMap.end()) {
        throw std::invalid_argument("Unsupported weather/time combination");
    }

    double lux = it->second;
    double speed, distance;

    if (time == TimeOfDay::Day) {
        if (lux >= 80000) { speed = 100; distance = 50; }
        else if (lux >= 10000) { speed = 90; distance = 45; }
        else if (lux >= 3000) { speed = 70; distance = 40; }
        else {
            if (lux <= 500) { speed = 40; distance = 60; }
            else if (lux <= 1500) { speed = 50; distance = 55; }
            else { speed = 60; distance = 50; }
        }
    }
    else {
        if (weather == WeatherCondition::LitBridge) {
            speed = 80; distance = 45;
        }
        else if (lux >= 0.15) {
            speed = 60; distance = 50;
        }
        else {
            speed = 40; distance = 60;
        }
    }

    return { lux, speed, distance };
}

// 天气效果管理器实现
WeatherEffectManager::WeatherEffectManager(int winWidth, int winHeight)
    : windowWidth(winWidth), windowHeight(winHeight), currentWeather(NOTHING) {
    // 动态分配数组
    rain = new Particle[RAIN_COUNT];
    snow = new Particle[SNOW_COUNT];
    
    // 初始化雨滴位置（从顶部菜单栏下方开始）
    for (int i = 0; i < RAIN_COUNT; ++i) {
        rain[i].x = rand() % windowWidth;
        rain[i].y = 80 + (rand() % (windowHeight - 80)); // 避开顶部80像素的菜单栏
    }
    // 初始化雪花位置（从顶部菜单栏下方开始）
    for (int i = 0; i < SNOW_COUNT; ++i) {
        snow[i].x = rand() % windowWidth;
        snow[i].y = 80 + (rand() % (windowHeight - 80)); // 避开顶部80像素的菜单栏
    }
}

WeatherEffectManager::~WeatherEffectManager() {
    delete[] rain;
    delete[] snow;
}

void WeatherEffectManager::setWeather(WeatherMode mode) {
    currentWeather = mode;
}

void WeatherEffectManager::update() {
    if (currentWeather == RAIN) {
        // 绘制雨滴
        setlinecolor(RGB(135, 206, 250));
        for (int i = 0; i < RAIN_COUNT; ++i) {
            rain[i].y += 10;
            if (rain[i].y > windowHeight) {
                rain[i].y = 80; // 从顶部菜单栏下方重新开始
                rain[i].x = rand() % windowWidth;
            }
            // 只在道路区域绘制雨滴，避开顶部菜单栏
            if (rain[i].y >= 80) {
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
            
            if (snow[i].y < 90) {
                snow[i].y = 90; // 确保雪花不会进入菜单栏区域
            }

            if (snow[i].x < 0) snow[i].x = 0;
            if (snow[i].x > windowWidth) snow[i].x = windowWidth;

            // 只在道路区域绘制雪花，避开顶部菜单栏
            if (snow[i].y >= 90) {
                fillcircle((int)snow[i].x, (int)snow[i].y, 2);
            }
        }
    }
}