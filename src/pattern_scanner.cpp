#include "../include/pattern_scanner.h"
#include <sstream>
#include <iomanip>
#include "../include/logger.h" // Include the Logger header file

// Known WoW function patterns
const std::vector<PatternScanner::Pattern> WOW_PATTERNS = {
    PatternScanner::CreatePattern(
        "Lua_DoString",
        "55 8B EC 83 EC ? 53 56 57 8B 7D ? 85 FF 0F 84 ? ? ? ? 8B 35",
        0x0),
    PatternScanner::CreatePattern(
        "Lua_GetTop",
        "55 8B EC 51 56 8B 75 ? 85 F6 74 ? 8B 46",
        0x0),
    PatternScanner::CreatePattern(
        "CGGameUI__Target",
        "55 8B EC 83 E4 ? 83 EC ? 53 56 57 8B F9 8B 87",
        0x0),
    PatternScanner::CreatePattern(
        "Script_CastSpellByName",
        "55 8B EC 83 EC ? 53 8B 5D ? 56 57 8B 7D ? 85 FF",
        0x0),
    PatternScanner::CreatePattern(
        "FrameScript__Execute",
        "55 8B EC 83 EC ? 53 8B 5D ? 56 57 8B F9 85 DB 0F 84",
        0x0)};

// Test implementations
bool PatternScanner::TestBasicPatternMatching()
{
    auto testPattern = CreatePattern("test", "AA BB CC DD EE", 0);
    std::vector<BYTE> testBytes = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
    return ScanRegion(MEMORY_BASIC_INFORMATION{}, testPattern).has_value();
}

bool PatternScanner::TestWildcardPatterns()
{
    auto testPattern = CreatePattern("wildcard", "AA ?? CC DD ?E", 0);
    std::vector<BYTE> testBytes = {0xAA, 0x11, 0xCC, 0xDD, 0xFE};
    return ScanRegion(MEMORY_BASIC_INFORMATION{}, testPattern).has_value();
}

bool PatternScanner::TestOffsetCalculation()
{
    auto testPattern = CreatePattern("offset", "AA BB CC", 5);
    auto result = ScanRegion(MEMORY_BASIC_INFORMATION{}, testPattern);
    return result.has_value() && (result->address == (uintptr_t)100 + testPattern.offset);
}

uintptr_t PatternScanner::FindPattern(const char *pattern, const char *, const char *)
{
    auto pat = CreatePattern("", pattern, 0);
    auto results = ScanProcess({pat});
    return !results.empty() ? results[0].address : 0;
}

PatternScanner::PatternScanner(HANDLE process) : processHandle(process)
{
    GetProcessMemoryRegions();
}

void PatternScanner::GetProcessMemoryRegions()
{
    MEMORY_BASIC_INFORMATION mbi;
    LPVOID address = nullptr;

    while (VirtualQueryEx(processHandle, address, &mbi, sizeof(mbi)))
    {
        if ((mbi.State == MEM_COMMIT) &&
            ((mbi.Protect & PAGE_GUARD) == 0) &&
            ((mbi.Protect & PAGE_NOACCESS) == 0) &&
            ((mbi.Type & MEM_PRIVATE) == 0))
        {

            memoryRegions.push_back(mbi);
        }
        address = (LPVOID)((DWORD_PTR)mbi.BaseAddress + mbi.RegionSize);
    }
}

std::vector<std::pair<BYTE, bool>> PatternScanner::ParsePattern(const std::string &pattern)
{

    Logger::Debug("Parsing pattern: " + pattern);

    std::vector<std::pair<BYTE, bool>> result;
    std::istringstream iss(pattern);
    std::string byte;

    while (iss >> byte)
    {
        if (byte == "?")
        {
            result.push_back({0, false});
        }
        else
        {
            try
            {
                result.push_back({(BYTE)std::stoi(byte, nullptr, 16), true});
            }
            catch (const std::invalid_argument &)
            {
                // Handle the error or log it
                Logger::Error("Invalid byte value in pattern: " + byte);
                result.push_back({0, false});
            }
            catch (const std::out_of_range &)
            {
                // Handle the error or log it
                Logger::Error("Invalid byte value in pattern: " + byte);
                result.push_back({0, false});
            }
        }
    }
    return result;
}

PatternScanner::Pattern PatternScanner::CreatePattern(
    const std::string &name,
    const std::string &pattern,
    int32_t offset)
{
    return {name, ParsePattern(pattern), offset};
}

std::optional<PatternScanner::ScanResult> PatternScanner::ScanRegion(
    const MEMORY_BASIC_INFORMATION &region,
    const Pattern &pattern)
{
    std::vector<BYTE> buffer(region.RegionSize);
    SIZE_T bytesRead;

    if (!ReadProcessMemory(processHandle, region.BaseAddress, buffer.data(), region.RegionSize, &bytesRead))
    {
        return std::nullopt;
    }

    for (size_t i = 0; i < buffer.size() - pattern.bytes.size(); ++i)
    {
        bool found = true;
        for (size_t j = 0; j < pattern.bytes.size(); ++j)
        {
            if (pattern.bytes[j].second && // if not wildcard
                buffer[i + j] != pattern.bytes[j].first)
            {
                found = false;
                break;
            }
        }

        if (found)
        {
            ScanResult result;
            result.address = (uintptr_t)region.BaseAddress + i + pattern.offset;
            result.patternName = pattern.name;
            result.bytes.assign(
                buffer.begin() + i,
                buffer.begin() + i + pattern.bytes.size());
            return result;
        }
    }

    return std::nullopt;
}

std::vector<PatternScanner::ScanResult> PatternScanner::ScanProcess(
    const std::vector<Pattern> &patterns)
{
    std::vector<ScanResult> results;

    for (const auto &region : memoryRegions)
    {
        for (const auto &pattern : patterns)
        {
            auto result = ScanRegion(region, pattern);
            if (result)
            {
                results.push_back(*result);
            }
        }
    }

    return results;
}