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
           
        _         __  ______  _ _     _   
       | |    __ _\ \/ / __ )| (_)___| |_ 
       | |   / _` |\  /|  _ \| | / __| __|
       | |__| (_| |/  \| |_) | | \__ \ |_ 
       |_____\__,_/_/\_\____/|_|_|___/\__|
                                        

              Version %s (%s)                       
              Build: %s - %s      



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
        Logger::Info("Starting tests: " + CliParser::TestModeToString(options.testMode));

        bool allPassed = true;
        LuaInjector injector;

        switch (options.testMode)
        {
        case CliParser::TestMode::ANTI_DEBUG:
        {
            Logger::Info("Running Anti-Debug test...");
            bool result = IsDebuggerPresent() == FALSE;
            if (result)
            {
                Logger::Info("Anti-Debug test: PASSED");
            }
            else
            {
                Logger::Error("Anti-Debug test: FAILED");
                allPassed = false;
            }
            break;
        }

        case CliParser::TestMode::PATTERN_SCAN:
        {
            Logger::Info("Running Pattern Scanning test...");
            bool result = injector.TestPatternScanning();
            if (result)
            {
                Logger::Info("Pattern Scanning test: PASSED");
            }
            else
            {
                Logger::Error("Pattern Scanning test: FAILED");
                allPassed = false;
            }
            break;
        }

        case CliParser::TestMode::IAT_HIDING:
        {
            Logger::Info("Running IAT Hiding test...");
            bool result = injector.TestIATHiding();
            if (result)
            {
                Logger::Info("IAT Hiding test: PASSED");
            }
            else
            {
                Logger::Error("IAT Hiding test: FAILED");
                allPassed = false;
            }
            break;
        }

        case CliParser::TestMode::WARDEN_DETECTION:
        {
            Logger::Info("Running Warden Detection test...");
            bool result = injector.TestWardenDetection();
            if (result)
            {
                Logger::Info("Warden Detection test: PASSED");
            }
            else
            {
                Logger::Error("Warden Detection test: FAILED");
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
                Logger::Info("Anti-Debug test: PASSED");
            }
            else
            {
                Logger::Error("Anti-Debug test: FAILED");
                allPassed = false;
            }

            // Pattern Scanning Test
            bool patternResult = injector.TestPatternScanning();
            if (patternResult)
            {
                Logger::Info("Pattern Scanning test: PASSED");
            }
            else
            {
                Logger::Error("Pattern Scanning test: FAILED");
                allPassed = false;
            }

            // IAT Hiding Test
            bool iatResult = injector.TestIATHiding();
            if (iatResult)
            {
                Logger::Info("IAT Hiding test: PASSED");
            }
            else
            {
                Logger::Error("IAT Hiding test: FAILED");
                allPassed = false;
            }

            // Warden Detection Test
            bool wardenResult = injector.TestWardenDetection();
            if (wardenResult)
            {
                Logger::Info("Warden Detection test: PASSED");
            }
            else
            {
                Logger::Error("Warden Detection test: FAILED");
                allPassed = false;
            }
            break;
        }

        default:
            Logger::Error("Invalid test mode");
            return 1;
        }

        if (allPassed)
        {
            Logger::Info("All tests passed successfully");
            return 0;
        }
        else
        {
            Logger::Error("One or more tests failed");
            return 1;
        }
    }
}
int _process(int argc, char *argv[])
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
            Logger::Critical("This program requires administrator privileges");
            return 1;
        }

        // Run tests if requested
        if (options.testMode != CliParser::TestMode::NONE)
        {
            return RunTests(options);
        }

        // Normal injection mode
        Logger::Info("Starting injection process...");

        // Basic anti-debug check
        if (IsDebuggerPresent())
        {
            Logger::Warning("Debugger detected, exiting...");
            return 1;
        }

        // Create and configure injector
        LuaInjector injector;

        // Set custom script if provided
        if (!options.customLuaScript.empty())
        {
            if (!injector.SetCustomScript(options.customLuaScript))
            {
                Logger::Error("Failed to load custom script");
                return 1;
            }
        }

        Logger::Info("Attempting injection...");

        // Perform injection
        if (injector.Inject())
        {
            Logger::Info("Injection completed successfully");
        }
        else
        {
            Logger::Error("Injection failed");
            return 1;
        }
    }
    catch (const std::exception &e)
    {
        Logger::Critical(std::string("An error occurred: ") + e.what());
        return 1;
    }
    catch (...)
    {
        Logger::Critical("An unknown error occurred");
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int result = _process(argc, argv);
    if (result == 0)
    {
        std::cout << "Press any key to exit..." << std::endl;
        std::cin.get();
    }
    if (result == 1)
    {
        std::cout << "Press any key to exit..." << std::endl;
        std::cin.get();
    }
    return result;
}