#include "../include/lua_injector.h"
#include "../include/logger.h"
#include "../include/version.h"
#include "../include/cli_parser.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <windows.h>

namespace
{
    void PrintBanner()
    {
        const char *banner = R"(
╔══════════════════════════════════════════════════════════╗
║              WoW Lua Injector - Advanced                 ║
║              Version %s (%s)                        ║
║          Build: %s - %s                       ║
╚══════════════════════════════════════════════════════════╝
)";

        printf(banner, VERSION_STRING, ARCHITECTURE, BUILD_DATE, BUILD_CONFIG);

        // Print feature status
        std::cout << "\nFeatures:\n"
                  << "- Pattern Scanning: " << (FEATURE_PATTERN_SCANNING ? "Enabled" : "Disabled") << "\n"
                  << "- Warden Detection: " << (FEATURE_WARDEN_DETECTION ? "Enabled" : "Disabled") << "\n"
                  << "- IAT Hiding: " << (FEATURE_IAT_HIDING ? "Enabled" : "Disabled") << "\n"
                  << "- Thread Hijacking: " << (FEATURE_THREAD_HIJACKING ? "Enabled" : "Disabled") << "\n\n";
    }

    bool CheckAdminPrivileges()
    {
        BOOL isAdmin = FALSE;
        HANDLE tokenHandle;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tokenHandle))
        {
            TOKEN_ELEVATION elevation;
            DWORD size = sizeof(TOKEN_ELEVATION);
            if (GetTokenInformation(tokenHandle, TokenElevation, &elevation, sizeof(elevation), &size))
            {
                isAdmin = elevation.TokenIsElevated;
            }
            CloseHandle(tokenHandle);
        }
        return isAdmin;
    }

    int RunTests(const CliParser::Options &options)
    {
        Logging::Info("Starting tests: " + CliParser::TestModeToString(options.testMode));

        bool allPassed = true;
        LuaInjector injector;

        switch (options.testMode)
        {
        case CliParser::TestMode::ANTI_DEBUG:
        {
            Logging::Info("Running Anti-Debug test...");
            bool result = IsDebuggerPresent() == FALSE;
            if (result)
            {
                Logging::Info("Anti-Debug test: PASSED");
            }
            else
            {
                Logging::Error("Anti-Debug test: FAILED");
                allPassed = false;
            }
            break;
        }

        case CliParser::TestMode::PATTERN_SCAN:
        {
            Logging::Info("Running Pattern Scanning test...");
            bool result = injector.TestPatternScanning();
            if (result)
            {
                Logging::Info("Pattern Scanning test: PASSED");
            }
            else
            {
                Logging::Error("Pattern Scanning test: FAILED");
                allPassed = false;
            }
            break;
        }

        case CliParser::TestMode::IAT_HIDING:
        {
            Logging::Info("Running IAT Hiding test...");
            bool result = injector.TestIATHiding();
            if (result)
            {
                Logging::Info("IAT Hiding test: PASSED");
            }
            else
            {
                Logging::Error("IAT Hiding test: FAILED");
                allPassed = false;
            }
            break;
        }

        case CliParser::TestMode::WARDEN_DETECTION:
        {
            Logging::Info("Running Warden Detection test...");
            bool result = injector.TestWardenDetection();
            if (result)
            {
                Logging::Info("Warden Detection test: PASSED");
            }
            else
            {
                Logging::Error("Warden Detection test: FAILED");
                allPassed = false;
            }
            break;
        }

        case CliParser::TestMode::ALL:
        {
            // Anti-Debug Test
            bool antiDebugResult = IsDebuggerPresent() == FALSE;
            if (antiDebugResult)
            {
                Logging::Info("Anti-Debug test: PASSED");
            }
            else
            {
                Logging::Error("Anti-Debug test: FAILED");
                allPassed = false;
            }

            // Pattern Scanning Test
            bool patternResult = injector.TestPatternScanning();
            if (patternResult)
            {
                Logging::Info("Pattern Scanning test: PASSED");
            }
            else
            {
                Logging::Error("Pattern Scanning test: FAILED");
                allPassed = false;
            }

            // IAT Hiding Test
            bool iatResult = injector.TestIATHiding();
            if (iatResult)
            {
                Logging::Info("IAT Hiding test: PASSED");
            }
            else
            {
                Logging::Error("IAT Hiding test: FAILED");
                allPassed = false;
            }

            // Warden Detection Test
            bool wardenResult = injector.TestWardenDetection();
            if (wardenResult)
            {
                Logging::Info("Warden Detection test: PASSED");
            }
            else
            {
                Logging::Error("Warden Detection test: FAILED");
                allPassed = false;
            }
            break;
        }

        default:
            Logging::Error("Invalid test mode");
            return 1;
        }

        if (allPassed)
        {
            Logging::Info("All tests passed successfully");
            return 0;
        }
        else
        {
            Logging::Error("One or more tests failed");
            return 1;
        }
    }
}

int main(int argc, char *argv[])
{
    try
    {
        // Parse command line arguments
        auto options = CliParser::Parse(argc, argv);

        // Set console title
        SetConsoleTitleA(FULL_VERSION_STRING);

        // Print banner
        PrintBanner();

        // Show help if requested
        if (options.help)
        {
            CliParser::PrintUsage();
            return 0;
        }

        // Check admin privileges
        if (!CheckAdminPrivileges())
        {
            Logging::Critical("This program requires administrator privileges");
            return 1;
        }

        // Run tests if requested
        if (options.testMode != CliParser::TestMode::NONE)
        {
            return RunTests(options);
        }

        // Normal injection mode
        Logging::Info("Starting injection process...");

        // Basic anti-debug check
        if (IsDebuggerPresent())
        {
            Logging::Warning("Debugger detected, exiting...");
            return 1;
        }

        // Create and configure injector
        LuaInjector injector;

        // Set custom script if provided
        if (!options.customLuaScript.empty())
        {
            if (!injector.SetCustomScript(options.customLuaScript))
            {
                Logging::Error("Failed to load custom script");
                return 1;
            }
        }

        Logging::Info("Attempting injection...");

        // Perform injection
        if (injector.Inject())
        {
            Logging::Info("Injection completed successfully");
        }
        else
        {
            Logging::Error("Injection failed");
            return 1;
        }
    }
    catch (const std::exception &e)
    {
        Logging::Critical(std::string("An error occurred: ") + e.what());
        return 1;
    }
    catch (...)
    {
        Logging::Critical("An unknown error occurred");
        return 1;
    }

    return 0;
}