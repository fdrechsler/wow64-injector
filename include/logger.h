#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <windows.h>

namespace Logging
{
    enum class LogLevel
    {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };

    inline void Log(LogLevel level, const std::string &message)
    {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

        // Get console handle for color output
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        // Set color based on level
        WORD color;
        const char *prefix;
        switch (level)
        {
        case LogLevel::DEBUG:
            color = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            prefix = "DEBUG";
            break;
        case LogLevel::INFO:
            color = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            prefix = "INFO";
            break;
        case LogLevel::WARNING:
            color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            prefix = "WARNING";
            break;
        case LogLevel::ERROR:
            color = FOREGROUND_RED | FOREGROUND_INTENSITY;
            prefix = "ERROR";
            break;
        case LogLevel::CRITICAL:
            color = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            prefix = "CRITICAL";
            break;
        default:
            color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            prefix = "UNKNOWN";
            break;
        }

        // Save current attributes
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);

        // Set color
        SetConsoleTextAttribute(hConsole, color);

        // Print log message
        std::cout << "[" << oss.str() << "] [" << std::setw(8) << prefix << "] "
                  << message << std::endl;

        // Restore original attributes
        SetConsoleTextAttribute(hConsole, csbi.wAttributes);
    }

    inline void Debug(const std::string &message) { Log(LogLevel::DEBUG, message); }
    inline void Info(const std::string &message) { Log(LogLevel::INFO, message); }
    inline void Warning(const std::string &message) { Log(LogLevel::WARNING, message); }
    inline void Error(const std::string &message) { Log(LogLevel::ERROR, message); }
    inline void Critical(const std::string &message) { Log(LogLevel::CRITICAL, message); }
}