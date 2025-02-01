#pragma once
#include "warden_detector.h"
#include "pattern_scanner.h"
#include "iat_hiding.h"
#include "process_memory_handler.h"
#include <windows.h>
#include <string>
#include <memory>
#include <functional>

class LuaInjector
{
public:
    LuaInjector();
    ~LuaInjector();

    // Main injection functionality
    bool Inject();
    bool SetCustomScript(const std::string &scriptPath);

    // Test functionality
    bool TestPatternScanning();
    bool TestIATHiding();
    bool TestWardenDetection();

private:
    HANDLE hProcess;
    DWORD processId;
    WardenDetector wardenDetector;
    std::unique_ptr<ProcessMemoryHandler> memoryHandler;
    std::string customScriptPath;

    struct WowOffsets
    {
        uintptr_t Lua_DoString;
        uintptr_t Lua_GetTop;
        uintptr_t CGGameUI__Target;
        uintptr_t Script_CastSpellByName;
        uintptr_t FrameScript__Execute;
    } offsets;

    std::vector<BYTE> EncryptPayload(const char *payload);
    bool FindProcess(const wchar_t *processName);
    bool FindOffsets();
    bool IsBeingDebugged();
    bool IsVirtualized();
    bool AllocateAndInject(const std::vector<BYTE> &payload);
    void CleanupInjection();
    bool HijackGameThread(LPVOID shellcodeAddress);
    DWORD FindMainGameThread();

    // Test helper methods
    bool TestMemoryPatterns();
    bool TestProcessAttach();
    bool TestThreadManipulation();
    bool VerifyInjectedCode();

    static const BYTE XOR_KEY = 0x55;
    static const char *DEFAULT_LUA_PAYLOAD;
    static const BYTE SHELLCODE[];
    static const size_t SHELLCODE_SIZE;
};