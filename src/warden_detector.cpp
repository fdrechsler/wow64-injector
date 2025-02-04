#include "../include/warden_detector.h"

// Known Warden signatures
const std::vector<std::vector<BYTE>> WARDEN_SIGNATURES = {
    {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x14, 0x53, 0x56, 0x57},
    {0x8B, 0xFF, 0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x20, 0x64},
    {0x6A, 0x00, 0x6A, 0x00, 0x68, 0x00, 0x00, 0x00, 0x00}};

// Warden module names (encrypted)
const std::vector<std::string> WARDEN_MODULES = {
    "\x57\x61\x72\x64\x65\x6E\x2E\x64\x6C\x6C", // "Warden.dll"
    "\x77\x6F\x77\x2E\x65\x78\x65",             // "wow.exe"
    "\x77\x6F\x77\x2D\x36\x34\x2E\x65\x78\x65"  // "wow-64.exe"
};

// Timing patterns for Warden scans
std::vector<WardenDetector::TimingPattern> WARDEN_TIMING = {
    {15000, 500, 3},  // Pattern A: 15s ±500ms, 3 times
    {30000, 1000, 2}, // Pattern B: 30s ±1s, 2 times
    {60000, 2000, 1}  // Pattern C: 60s ±2s, once
};

void WardenDetector::InitializeWardenDetection(HANDLE process)
{
    hProcess = process;
    AnalyzeMemoryPermissions();
}

bool WardenDetector::ScanMemoryRegion(const MemoryRegion &region)
{
    std::vector<BYTE> buffer(region.Size);
    SIZE_T bytesRead;

    if (!ReadProcessMemory(hProcess, region.BaseAddress, buffer.data(), region.Size, &bytesRead))
    {
        return false;
    }

    for (const auto &signature : WARDEN_SIGNATURES)
    {
        for (int i = 0; i < static_cast<int>(buffer.size() - signature.size()); i++)
        {
            if (memcmp(buffer.data() + i, signature.data(), signature.size()) == 0)
            {
                return true; // Warden signature found
            }
        }
    }
    return false;
}

bool WardenDetector::MonitorThreadActivity()
{
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE)
        return false;

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);
    std::vector<DWORD> threadTimes;

    if (Thread32First(hThreadSnap, &te32))
    {
        do
        {
            if (te32.th32OwnerProcessID == GetProcessId(hProcess))
            {
                HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, te32.th32ThreadID);
                if (hThread)
                {
                    FILETIME creation, exit, kernel, user;
                    if (GetThreadTimes(hThread, &creation, &exit, &kernel, &user))
                    {
                        ULARGE_INTEGER userTime;
                        userTime.LowPart = user.dwLowDateTime;
                        userTime.HighPart = user.dwHighDateTime;
                        threadTimes.push_back((DWORD)(userTime.QuadPart / 10000)); // Convert to milliseconds
                    }
                    CloseHandle(hThread);
                }
            }
        } while (Thread32Next(hThreadSnap, &te32));
    }

    CloseHandle(hThreadSnap);

    // Analyze thread timing patterns
    for (const auto &pattern : WARDEN_TIMING)
    {
        int matchCount = 0;
        for (size_t i = 1; i < threadTimes.size(); i++)
        {
            DWORD interval = threadTimes[i] - threadTimes[i - 1];
            if (abs((long)(interval - pattern.interval)) <= pattern.variation)
            {
                matchCount++;
            }
        }
        if (matchCount >= pattern.count)
        {
            return true; // Warden timing pattern detected
        }
    }
    return false;
}

bool WardenDetector::AnalyzeMemoryPermissions()
{
    MEMORY_BASIC_INFORMATION mbi;
    LPVOID address = NULL;

    while (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)))
    {
        if (mbi.State == MEM_COMMIT)
        {
            MemoryRegion region;
            region.BaseAddress = mbi.BaseAddress;
            region.Size = mbi.RegionSize;
            region.Protection = mbi.Protect;
            memoryRegions.push_back(region);

            if ((mbi.Protect & PAGE_EXECUTE_READWRITE) ||
                (mbi.Protect & PAGE_EXECUTE_WRITECOPY))
            {
                return true; // Suspicious memory permissions found
            }
        }
        address = (LPVOID)((DWORD_PTR)mbi.BaseAddress + mbi.RegionSize);
    }
    return false;
}

bool WardenDetector::CheckModuleIntegrity()
{
    HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetProcessId(hProcess));
    if (hModuleSnap == INVALID_HANDLE_VALUE)
        return false;

    MODULEENTRY32 me32;
    me32.dwSize = sizeof(MODULEENTRY32);
    std::vector<std::string> loadedModules;

    if (Module32First(hModuleSnap, &me32))
    {
        do
        {
            char moduleName[MAX_PATH];
            WideCharToMultiByte(CP_ACP, 0, me32.szModule, -1, moduleName, MAX_PATH, NULL, NULL);
            loadedModules.push_back(moduleName);
        } while (Module32Next(hModuleSnap, &me32));
    }

    CloseHandle(hModuleSnap);

    // Check for Warden modules
    for (const auto &wardenMod : WARDEN_MODULES)
    {
        for (const auto &loadedMod : loadedModules)
        {
            if (loadedMod.find(wardenMod) != std::string::npos)
            {
                return true; // Warden module found
            }
        }
    }
    return false;
}

bool WardenDetector::IsWardenActive()
{
    return ScanMemoryRegion(memoryRegions[0]) ||
           MonitorThreadActivity() ||
           AnalyzeMemoryPermissions() ||
           CheckModuleIntegrity();
}

DWORD WINAPI WardenDetector::MonitorThread(LPVOID param)
{
    WardenDetector *detector = (WardenDetector *)param;
    while (true)
    {
        if (detector->IsWardenActive())
        {
            // Trigger evasion procedures
            break;
        }
        Sleep(100); // Check every 100ms
    }
    return 0;
}

// Tests
bool WardenDetector::TestSignatureScanning()
{
    // Test signature scanning
    std::vector<BYTE> buffer = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x14, 0x53, 0x56, 0x57};
    return ScanMemoryRegion({buffer.data(), buffer.size(), PAGE_EXECUTE_READWRITE});
}

bool WardenDetector::TestBehaviorAnalysis()
{
    // Test behavior analysis
    return MonitorThreadActivity();
}

bool WardenDetector::TestTimingChecks()
{
    // Test timing checks
    return AnalyzeMemoryPermissions();
}