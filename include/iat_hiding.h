#pragma once
#include <windows.h>
#include <winternl.h>
#include <vector>

class IATHider
{
public:
    static void HideIAT();
    static void RestoreIAT();

    // Test methods
    static bool TestHidingMechanism();
    static bool TestRestoration();

private:
    struct ImportBackup
    {
        LPCSTR FunctionName;
        PVOID OriginalAddress;
        PVOID HookedAddress;
    };

    static std::vector<ImportBackup> backupEntries;
    static PVOID GetModuleBaseAddress();
    static bool IsAddressInIAT(PVOID address);
    static PVOID GetProcAddressManually(LPCSTR moduleName, LPCSTR functionName);
    static void EncryptIAT();
    static void DecryptIAT();
    static void BackupImports();
    static void RestoreImports();
    static void ModifyMemoryProtection(PVOID address, SIZE_T size, DWORD newProtection, PDWORD oldProtection);
};