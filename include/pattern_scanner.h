#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <optional>

class PatternScanner
{
public:
    struct Pattern
    {
        std::string name;
        std::vector<std::pair<BYTE, bool>> bytes; // byte value and mask flag
        int32_t offset;
    };

    struct ScanResult
    {
        uintptr_t address;
        std::string patternName;
        std::vector<BYTE> bytes;
    };

    PatternScanner(HANDLE process);
    std::vector<ScanResult> ScanProcess(const std::vector<Pattern> &patterns);
    static Pattern CreatePattern(const std::string &name, const std::string &pattern, int32_t offset = 0);

private:
    HANDLE processHandle;
    std::vector<MEMORY_BASIC_INFORMATION> memoryRegions;

    void GetProcessMemoryRegions();
    std::optional<ScanResult> ScanRegion(const MEMORY_BASIC_INFORMATION &region, const Pattern &pattern);
    static std::vector<std::pair<BYTE, bool>> ParsePattern(const std::string &pattern);
};