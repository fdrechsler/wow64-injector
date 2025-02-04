#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <windows.h>

class Logger
{
public:
    static void Debug(const std::string &message)
    {
        LogWithColor(message, FOREGROUND_BLUE | FOREGROUND_INTENSITY, "DEBUG");
    }

    static void Info(const std::string &message)
    {
        LogWithColor(message, FOREGROUND_GREEN | FOREGROUND_INTENSITY, "INFO");
    }

    static void Warning(const std::string &message)
    {
        LogWithColor(message, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "WARNING");
    }

    static void Error(const std::string &message)
    {
        LogWithColor(message, FOREGROUND_RED | FOREGROUND_INTENSITY, "ERROR");
    }

    static void Critical(const std::string &message)
    {
        LogWithColor(message, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY, "CRITICAL");
    }

private:
    static void LogWithColor(const std::string &message, WORD color, const char *level)
    {
        auto t = std::time(nullptr);
        std::tm tm;
        localtime_s(&tm, &t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);

        SetConsoleTextAttribute(hConsole, color);
        std::cout << "[" << oss.str() << "] [" << std::setw(8) << level << "] "
                  << message << std::endl;
        SetConsoleTextAttribute(hConsole, csbi.wAttributes);
    }
};