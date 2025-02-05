#include "../include/lua_injector.h"
#include <tlhelp32.h>
#include "logger.h" // Include the appropriate header for Logger

const char *LuaInjector::DEFAULT_LUA_PAYLOAD = R"(
local function checkWarden()
    local t1 = GetTime()
    local t2 = GetTime()
    if (t2 - t1) < 0.001 then
        return true -- Warden scan detected
    end
    return false
end

local function main()
    if checkWarden() then return end
    -- Your Lua code here
    print("Injection successful!")
end

C_Timer.After(0.1, main)
 C_Timer.NewTimer(3, function() print("Hello X") end)
)";

// Shellcode to execute Lua_DoString
#ifdef _WIN64
const BYTE LuaInjector::SHELLCODE[] = {
    0x48, 0x89, 0x4C, 0x24, 0x08,                               // mov [rsp+8], rcx
    0x48, 0x83, 0xEC, 0x28,                                     // sub rsp, 28h
    0x48, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rcx, Lua_DoString
    0xFF, 0xD1,                                                 // call rcx
    0x48, 0x83, 0xC4, 0x28,                                     // add rsp, 28h
    0xC3                                                        // ret
};
#else
const BYTE LuaInjector::SHELLCODE[] = {
    0x55,                               // push ebp
    0x8B, 0xEC,                         // mov ebp, esp
    0x83, 0xEC, 0x08,                   // sub esp, 8
    0x53,                               // push ebx
    0x56,                               // push esi
    0x57,                               // push edi
    0x8B, 0x7D, 0x08,                   // mov edi, [ebp+8]
    0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, // call Lua_DoString
    0x5F,                               // pop edi
    0x5E,                               // pop esi
    0x5B,                               // pop ebx
    0x8B, 0xE5,                         // mov esp, ebp
    0x5D,                               // pop ebp
    0xC3                                // ret
};
#endif

const size_t LuaInjector::SHELLCODE_SIZE = sizeof(SHELLCODE);

LuaInjector::LuaInjector() : hProcess(NULL), processId(0) {}

LuaInjector::~LuaInjector()
{
    CleanupInjection();
}
std::vector<BYTE> LuaInjector::ConvertCharPointerToVector(const char *payload)
{
    std::vector<BYTE> vectorbytes;
    size_t len = strlen(payload);
    vectorbytes.resize(len);

    for (size_t i = 0; i < len; i++)
    {
        vectorbytes[i] = payload[i];
    }
    return vectorbytes;
}
std::vector<BYTE> LuaInjector::EncryptPayload(const char *payload)
{
    std::vector<BYTE> encrypted;
    size_t len = strlen(payload);
    encrypted.resize(len);

    for (size_t i = 0; i < len; i++)
    {
        // XOR + nibble swap
        BYTE b = payload[i] ^ XOR_KEY;
        encrypted[i] = ((b << 4) | (b >> 4));
    }
    return encrypted;
}

bool LuaInjector::FindProcess(const wchar_t *processName)
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
        return false;

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(pe32);

    if (!Process32FirstW(snapshot, &pe32))
    {
        CloseHandle(snapshot);
        return false;
    }

    do
    {
        if (_wcsicmp(pe32.szExeFile, processName) == 0)
        {
            processId = pe32.th32ProcessID;
            CloseHandle(snapshot);
            return true;
        }
    } while (Process32NextW(snapshot, &pe32));

    CloseHandle(snapshot);
    return false;
}

DWORD LuaInjector::FindMainGameThread()
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
        return 0;

    THREADENTRY32 te32;
    te32.dwSize = sizeof(te32);
    DWORD mainThreadId = 0;
    ULONG_PTR lowestStack = (ULONG_PTR)-1;

    if (Thread32First(snapshot, &te32))
    {
        do
        {
            if (te32.th32OwnerProcessID == processId)
            {
                HANDLE thread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
                if (thread)
                {
                    CONTEXT context;
                    context.ContextFlags = CONTEXT_CONTROL;
                    if (GetThreadContext(thread, &context))
                    {
                        // Get stack pointer based on architecture
                        ULONG_PTR stackPtr;
#ifdef _WIN64
                        stackPtr = context.Rsp;
#else
                        stackPtr = context.Esp;
#endif
                        // Look for thread with lowest stack address (usually main thread)
                        if (stackPtr < lowestStack)
                        {
                            lowestStack = stackPtr;
                            mainThreadId = te32.th32ThreadID;
                        }
                    }
                    CloseHandle(thread);
                }
            }
        } while (Thread32Next(snapshot, &te32));
    }

    CloseHandle(snapshot);
    return mainThreadId;
}

bool LuaInjector::HijackGameThread(LPVOID shellcodeAddress)
{
    DWORD mainThreadId = FindMainGameThread();
    if (!mainThreadId)
        return false;

    return memoryHandler->HijackThread(mainThreadId, shellcodeAddress);
}

bool LuaInjector::AllocateAndInject(const std::vector<BYTE> &payload)
{
    // Allocate memory for payload using pattern-based allocation
    LPVOID payloadAddress = memoryHandler->AllocateMemory(
        payload.size(),
        PAGE_READWRITE,
        ProcessMemoryHandler::MemoryAllocationType::MEM_COMMIT_PATTERN);

    if (!payloadAddress)
    {
        Logger::Error("Failed to allocate memory for payload");
        return false;
    }

    Logger::Info("Allocated memory for payload at: " + std::to_string((uintptr_t)payloadAddress));
    // Write encrypted payload
    SIZE_T written;
    if (!memoryHandler->WriteMemory(payloadAddress, payload.data(), payload.size(), &written))
    {
        Logger::Error("Failed to write payload to allocated memory");
        return false;
    }

    Logger::Info("Wrote payload to allocated memory: " + std::to_string(written) + " bytes");

    // Allocate memory for shellcode
    LPVOID shellcodeAddress = memoryHandler->AllocateMemory(
        SHELLCODE_SIZE,
        PAGE_READWRITE);

    Logger::Info("Allocated memory for shellcode at: " + std::to_string((uintptr_t)shellcodeAddress));

    if (!shellcodeAddress)
    {
        Logger::Error("Failed to allocate memory for shellcode");
        return false;
    }

    // Prepare shellcode with proper address
    std::vector<BYTE> modifiedShellcode(SHELLCODE, SHELLCODE + SHELLCODE_SIZE);
#ifdef _WIN64
    *(uintptr_t *)(&modifiedShellcode[11]) = offsets.Lua_DoString;
#else
    *(uintptr_t *)(&modifiedShellcode[12]) = offsets.Lua_DoString;
#endif

    // call Lua_DoString

#ifdef _WIN64
    *(uintptr_t *)(&modifiedShellcode[11]) = offsets.FrameScript__Execute;
#else
    *(uintptr_t *)(&modifiedShellcode[11]) = offsets.FrameScript__Execute;
#endif

    // CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)offsets.FrameScript__Execute, NULL, 0, NULL);

    // Write shellcode
    if (!memoryHandler->WriteMemory(shellcodeAddress, modifiedShellcode.data(), SHELLCODE_SIZE))
    {

        Logger::Error("Failed to write shellcode to allocated memory");
        return false;
    }

    Logger::Info("Wrote shellcode to allocated memory");
    // Change shellcode protection to executable
    DWORD oldProtect;
    if (!memoryHandler->ProtectMemory(shellcodeAddress, SHELLCODE_SIZE, PAGE_EXECUTE_READ, &oldProtect))
    {
        Logger::Error("Failed to change shellcode memory protection");
        return false;
    }
    CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)offsets.FrameScript__Execute, NULL, 0, NULL);

    Logger::Info("Shellcode memory protection changed to PAGE_EXECUTE_READ");

    // Hijack thread to execute shellcode
    return HijackGameThread(shellcodeAddress);
}

bool LuaInjector::Inject()
{
    // Hide IAT before any Windows API calls
    IATHider::HideIAT();
    Logger::Info("IAT hidden");

    /*if (IsBeingDebugged() || IsVirtualized())
    {
        Logger::Error("Injection failed: debugger or virtualization detected");
        IATHider::RestoreIAT();
        return false;
    }*/

    if (!FindProcess(L"Wow.exe"))
    {
        Logger::Error("Failed to find WoW process");
        IATHider::RestoreIAT();
        return false;
    }

    Logger::Info("Found WoW process with PID: " + std::to_string(processId));
    // Open process with required access
    hProcess = OpenProcess(
        PROCESS_CREATE_THREAD |
            PROCESS_QUERY_INFORMATION |
            PROCESS_VM_OPERATION |
            PROCESS_VM_WRITE |
            PROCESS_VM_READ,
        FALSE,
        processId);

    Logger::Info("Process handle: " + std::to_string((uintptr_t)hProcess));

    if (!hProcess)
    {
        Logger::Error("Failed to open process");
        IATHider::RestoreIAT();
        return false;
    }

    // Initialize memory handler
    memoryHandler = std::make_unique<ProcessMemoryHandler>(hProcess);

    // Initialize Warden detection
    wardenDetector.InitializeWardenDetection(hProcess);

    // Find required function offsets
    if (!FindOffsets())
    {
        CleanupInjection();
        IATHider::RestoreIAT();
        return false;
    }

    // Prepare and inject payload
    const char *scriptToUse = customScriptPath.empty() ? DEFAULT_LUA_PAYLOAD : customScriptPath.c_str();
    // auto encrypted = EncryptPayload(scriptToUse);
    auto encrypted = ConvertCharPointerToVector(scriptToUse);
    bool success = AllocateAndInject(encrypted);

    // Cleanup
    // CleanupInjection();
    // IATHider::RestoreIAT();

    return success;
}

void LuaInjector::CleanupInjection()
{
    if (memoryHandler)
    {
        memoryHandler.reset();
    }
    if (hProcess)
    {
        CloseHandle(hProcess);
        hProcess = NULL;
    }
}

bool LuaInjector::IsBeingDebugged()
{
    BOOL isDebuggerPresent = FALSE;
    CheckRemoteDebuggerPresent(GetCurrentProcess(), &isDebuggerPresent);
    return isDebuggerPresent || IsDebuggerPresent();
}

bool LuaInjector::IsVirtualized()
{
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
    Sleep(1);
    QueryPerformanceCounter(&end);
    double elapsed = (end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;
    return elapsed < 0.9 || elapsed > 1.1;
}

bool LuaInjector::SetCustomScript(const std::string &scriptPath)
{
    customScriptPath = scriptPath;
    return !customScriptPath.empty();
}

bool LuaInjector::TestPatternScanning()
{
    PatternScanner scanner(hProcess);
    return scanner.TestBasicPatternMatching() &&
           scanner.TestWildcardPatterns() &&
           scanner.TestOffsetCalculation();
}

bool LuaInjector::TestIATHiding()
{
    return IATHider::TestHidingMechanism() &&
           IATHider::TestRestoration();
}

bool LuaInjector::TestWardenDetection()
{
    return wardenDetector.TestSignatureScanning() &&
           wardenDetector.TestBehaviorAnalysis() &&
           wardenDetector.TestTimingChecks();
}

bool LuaInjector::FindOffsets()
{
    PatternScanner scanner(hProcess);
    // Find Lua function offsets
    offsets.Lua_DoString = scanner.FindPattern(
        "48 89 ? 24 ? 48 83 EC",
        "xxxxxx????xx????",
        "Lua51.dll");

    if (offsets.Lua_DoString == 0)
    {
        Logger::Error("Failed to find Lua_DoString offset");
    }

    offsets.FrameScript__Execute = scanner.FindPattern("48 89 ? 24 ? 48 83 EC 28 ? ?", "xxxxxxxxxxxxxx", "FrameScript.dll");

    if (offsets.FrameScript__Execute == 0)
    {
        Logger::Error("Failed to find FrameScript__Execute offset");
    }
    Logger::Info("Found offsets: Lua_DoString: " + std::to_string(offsets.Lua_DoString) +
                 ", FrameScript__Execute: " + std::to_string(offsets.FrameScript__Execute));

    // call offset for FrameScript__Execute

    return offsets.Lua_DoString != 0 &&
           offsets.FrameScript__Execute != 0;
}