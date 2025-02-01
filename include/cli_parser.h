#pragma once
#include <string>
#include <vector>
#include <map>
#include <stdexcept>

class CliParser
{
public:
    enum class TestMode
    {
        NONE,
        ANTI_DEBUG,
        PATTERN_SCAN,
        IAT_HIDING,
        WARDEN_DETECTION,
        ALL
    };

    struct Options
    {
        TestMode testMode = TestMode::NONE;
        bool verbose = false;
        bool help = false;
        std::string customLuaScript;
    };

    static Options Parse(int argc, char *argv[])
    {
        Options options;
        std::vector<std::string> args(argv + 1, argv + argc);

        for (size_t i = 0; i < args.size(); ++i)
        {
            const std::string &arg = args[i];

            if (arg == "--help" || arg == "-h")
            {
                options.help = true;
                break;
            }
            else if (arg == "--verbose" || arg == "-v")
            {
                options.verbose = true;
            }
            else if (arg == "--test-anti-debug")
            {
                options.testMode = TestMode::ANTI_DEBUG;
            }
            else if (arg == "--test-pattern-scan")
            {
                options.testMode = TestMode::PATTERN_SCAN;
            }
            else if (arg == "--test-iat-hiding")
            {
                options.testMode = TestMode::IAT_HIDING;
            }
            else if (arg == "--test-warden")
            {
                options.testMode = TestMode::WARDEN_DETECTION;
            }
            else if (arg == "--test-all")
            {
                options.testMode = TestMode::ALL;
            }
            else if (arg == "--script" && i + 1 < args.size())
            {
                options.customLuaScript = args[++i];
            }
        }

        return options;
    }

    static void PrintUsage()
    {
        const char *usage = R"(
WoW Lua Injector - Usage:
-------------------------
Options:
  -h, --help              Show this help message
  -v, --verbose           Enable verbose logging
  --test-anti-debug       Test anti-debug functionality
  --test-pattern-scan     Test pattern scanning
  --test-iat-hiding       Test IAT hiding
  --test-warden          Test Warden detection
  --test-all             Run all tests
  --script <file>         Use custom Lua script file

Examples:
  injector32.exe                      # Normal injection
  injector32.exe --verbose            # Verbose mode
  injector32.exe --test-all          # Run all tests
  injector32.exe --script script.lua  # Use custom script

Note: This program requires administrator privileges.
)";
        printf("%s\n", usage);
    }

    static std::string TestModeToString(TestMode mode)
    {
        switch (mode)
        {
        case TestMode::NONE:
            return "None";
        case TestMode::ANTI_DEBUG:
            return "Anti-Debug";
        case TestMode::PATTERN_SCAN:
            return "Pattern Scanning";
        case TestMode::IAT_HIDING:
            return "IAT Hiding";
        case TestMode::WARDEN_DETECTION:
            return "Warden Detection";
        case TestMode::ALL:
            return "All Tests";
        default:
            return "Unknown";
        }
    }
};