#pragma once
#include <string>

// 天气模式枚举
enum WeatherMode { RAIN, SNOW, NOTHING };

// 天气条件枚举
enum class WeatherCondition {
    Sunny,
    Rain,
    Snow,
    SunnyNight,
    RainNight,
    SnowNight,
};

// 时间段枚举
enum class TimeOfDay {
    Day,
    Night
};

// 环境配置结构
struct EnvironmentConfig {
    double illuminance_lux;
};

// 桥梁交通控制器类
class BridgeTrafficController {
public:
    static double getEnvironmentConfig(TimeOfDay time, WeatherCondition weather);
};

// 天气效果管理器类
class WeatherEffectManager {
private:
    static const int RAIN_COUNT = 250;
    static const int SNOW_COUNT = 150;
    
    struct Particle {
        double x, y;
    };
    
    Particle* rain;
    Particle* snow;
    WeatherMode currentWeather;
    int windowWidth;
    int windowHeight;
    
public:
    WeatherEffectManager(int winWidth, int winHeight);
    ~WeatherEffectManager();
    void setWeather(WeatherMode mode);
    WeatherMode getCurrentWeather() const { return currentWeather; }
    void update();  // 更新并绘制天气效果
};
