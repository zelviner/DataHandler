#include <gtest/gtest.h>

#include <chrono>

TEST(Time, class) {
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();

    // 转换为时间点为 time_t 类型
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    // 转换为 tm 结构
    std::tm now_tm;
    localtime_s(&now_tm, &now_time_t); // Windows 平台

    // 获取年和月
    int year  = now_tm.tm_year + 1900; // tm_year 是从 1900 年起计数的
    int month = now_tm.tm_mon + 1;     // tm_mon 是从 0 开始的，所以要加 1

    std::cout << "当前时间的年: " << year << "\n";
    std::cout << "当前时间的月: " << month << "\n";
}