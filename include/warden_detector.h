#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <string>

class WardenDetector
{
public:
    struct MemoryRegion
    {
        PVOID BaseAddress;
        SIZE_T Size;
        DWORD Protection;
    };

    struct TimingPattern
    {
        DWORD interval;
        DWORD variation;
        DWORD count;
    };

    void InitializeWardenDetection(HANDLE process);
    bool IsWardenActive();
    static DWORD WINAPI MonitorThread(LPVOID param);

private:
    HANDLE hProcess;
    std::vector<MemoryRegion> memoryRegions;

    bool ScanMemoryRegion(const MemoryRegion &region);
    bool MonitorThreadActivity();
    bool AnalyzeMemoryPermissions();
    bool CheckModuleIntegrity();
};