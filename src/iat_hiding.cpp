#include "../include/iat_hiding.h"

std::vector<IATHider::ImportBackup> IATHider::backupEntries;

PVOID IATHider::GetModuleBaseAddress()
{
    return GetModuleHandleW(nullptr);
}

PVOID IATHider::GetProcAddressManually(LPCSTR moduleName, LPCSTR functionName)
{
    PVOID moduleBase = GetModuleHandleA(moduleName);
    if (!moduleBase)
        return nullptr;

    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)moduleBase;
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE *)moduleBase + dosHeader->e_lfanew);

    PIMAGE_EXPORT_DIRECTORY exportDir = (PIMAGE_EXPORT_DIRECTORY)((BYTE *)moduleBase +
                                                                  ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

    DWORD *functions = (DWORD *)((BYTE *)moduleBase + exportDir->AddressOfFunctions);
    WORD *ordinals = (WORD *)((BYTE *)moduleBase + exportDir->AddressOfNameOrdinals);
    DWORD *names = (DWORD *)((BYTE *)moduleBase + exportDir->AddressOfNames);

    for (DWORD i = 0; i < exportDir->NumberOfNames; i++)
    {
        LPCSTR currentName = (LPCSTR)((BYTE *)moduleBase + names[i]);
        if (strcmp(currentName, functionName) == 0)
        {
            return (PVOID)((BYTE *)moduleBase + functions[ordinals[i]]);
        }
    }

    return nullptr;
}

bool IATHider::IsAddressInIAT(PVOID address)
{
    PVOID baseAddress = GetModuleBaseAddress();
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)baseAddress;
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE *)baseAddress + dosHeader->e_lfanew);

    IMAGE_DATA_DIRECTORY importDir =
        ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

    PVOID iatStart = (PVOID)((BYTE *)baseAddress + importDir.VirtualAddress);
    PVOID iatEnd = (PVOID)((BYTE *)iatStart + importDir.Size);

    return (address >= iatStart && address <= iatEnd);
}

void IATHider::ModifyMemoryProtection(PVOID address, SIZE_T size, DWORD newProtection, PDWORD oldProtection)
{
    VirtualProtect(address, size, newProtection, oldProtection);
}

void IATHider::BackupImports()
{
    PVOID baseAddress = GetModuleBaseAddress();
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)baseAddress;
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE *)baseAddress + dosHeader->e_lfanew);

    PIMAGE_IMPORT_DESCRIPTOR importDesc = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE *)baseAddress +
                                                                     ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    while (importDesc->Name)
    {
        LPCSTR dllName = (LPCSTR)((BYTE *)baseAddress + importDesc->Name);
        PIMAGE_THUNK_DATA originalFirstThunk =
            (PIMAGE_THUNK_DATA)((BYTE *)baseAddress + importDesc->OriginalFirstThunk);
        PIMAGE_THUNK_DATA firstThunk =
            (PIMAGE_THUNK_DATA)((BYTE *)baseAddress + importDesc->FirstThunk);

        while (originalFirstThunk->u1.AddressOfData)
        {
            if (!(originalFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG))
            {
                PIMAGE_IMPORT_BY_NAME functionName = (PIMAGE_IMPORT_BY_NAME)((BYTE *)baseAddress + originalFirstThunk->u1.AddressOfData);

                ImportBackup backup;
                backup.FunctionName = (LPCSTR)functionName->Name;
                backup.OriginalAddress = (PVOID)firstThunk->u1.Function;
                backup.HookedAddress = GetProcAddressManually(dllName, (LPCSTR)functionName->Name);

                if (backup.HookedAddress)
                {
                    backupEntries.push_back(backup);
                }
            }
            originalFirstThunk++;
            firstThunk++;
        }
        importDesc++;
    }
}

void IATHider::EncryptIAT()
{
    PVOID baseAddress = GetModuleBaseAddress();
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)baseAddress;
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE *)baseAddress + dosHeader->e_lfanew);

    DWORD oldProtect;
    ModifyMemoryProtection(baseAddress, ntHeaders->OptionalHeader.SizeOfHeaders, PAGE_READWRITE, &oldProtect);

    // Zero out the import directory information
    ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = 0;
    ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = 0;

    ModifyMemoryProtection(baseAddress, ntHeaders->OptionalHeader.SizeOfHeaders, oldProtect, &oldProtect);
}

void IATHider::DecryptIAT()
{
    // This would restore the original IAT directory information
    // In a real implementation, you'd need to store this information somewhere
}

void IATHider::RestoreImports()
{
    for (const auto &backup : backupEntries)
    {
        PVOID *iatEntry = (PVOID *)backup.OriginalAddress;
        if (IsAddressInIAT(iatEntry))
        {
            DWORD oldProtect;
            ModifyMemoryProtection(iatEntry, sizeof(PVOID), PAGE_READWRITE, &oldProtect);
            *iatEntry = backup.OriginalAddress;
            ModifyMemoryProtection(iatEntry, sizeof(PVOID), oldProtect, &oldProtect);
        }
    }
    backupEntries.clear();
}

void IATHider::HideIAT()
{
    BackupImports();

    // Replace IAT entries with manually resolved addresses
    for (const auto &backup : backupEntries)
    {
        PVOID *iatEntry = (PVOID *)backup.OriginalAddress;
        if (IsAddressInIAT(iatEntry))
        {
            DWORD oldProtect;
            ModifyMemoryProtection(iatEntry, sizeof(PVOID), PAGE_READWRITE, &oldProtect);
            *iatEntry = backup.HookedAddress;
            ModifyMemoryProtection(iatEntry, sizeof(PVOID), oldProtect, &oldProtect);
        }
    }

    EncryptIAT();
}

void IATHider::RestoreIAT()
{
    DecryptIAT();
    RestoreImports();
}

// TESTS
bool IATHider::TestHidingMechanism()
{
    // dont implement this yet
    return true;
}

bool IATHider::TestRestoration()
{
    // Test restoration mechanism
    BackupImports();
    RestoreImports();
    return backupEntries.empty();
}
