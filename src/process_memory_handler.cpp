#include "../include/process_memory_handler.h"
#include <algorithm>
#include <tlhelp32.h>

ProcessMemoryHandler::ProcessMemoryHandler(HANDLE processHandle) : processHandle(processHandle)
{
    // Initialize NT API function pointers
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    NtGetContextThread = reinterpret_cast<NtGetContextThread_t>(
        GetProcAddress(ntdll, "NtGetContextThread"));
    NtSetContextThread = reinterpret_cast<NtSetContextThread_t>(
        GetProcAddress(ntdll, "NtSetContextThread"));
}

ProcessMemoryHandler::~ProcessMemoryHandler()
{
    // Cleanup allocated memory
    for (const auto &region : allocatedRegions)
    {
        VirtualFreeEx(processHandle, region.first, 0, MEM_RELEASE);
    }

    // Restore hijacked threads
    CleanupThreads();
}

LPVOID ProcessMemoryHandler::AllocateMemory(SIZE_T size, DWORD protection, MemoryAllocationType allocType)
{
    LPVOID address = nullptr;

    if (allocType == MemoryAllocationType::MEM_COMMIT_PATTERN)
    {
        if (!FindPatternAndAllocate(size, protection, address))
        {
            return nullptr;
        }
    }
    else
    {
        address = VirtualAllocEx(
            processHandle,
            nullptr,
            size,
            MEM_COMMIT | MEM_RESERVE,
            protection);
    }

    if (address)
    {
        allocatedRegions.push_back({address, size});
    }

    return address;
}

bool ProcessMemoryHandler::WriteMemory(LPVOID address, const void *buffer, SIZE_T size, SIZE_T *bytesWritten)
{
    if (!ValidateMemoryRegion(address, size))
    {
        return false;
    }

    // Create a temporary buffer for XOR masking
    std::vector<BYTE> maskedBuffer(static_cast<const BYTE *>(buffer),
                                   static_cast<const BYTE *>(buffer) + size);
    XorBuffer(maskedBuffer.data(), size);

    return WriteProcessMemory(
        processHandle,
        address,
        maskedBuffer.data(),
        size,
        bytesWritten);
}

bool ProcessMemoryHandler::HijackThread(DWORD threadId, LPVOID newAddress)
{
    HANDLE threadHandle = OpenThread(THREAD_ALL_ACCESS, FALSE, threadId);
    if (!threadHandle)
    {
        return false;
    }

    ThreadContext context;
    context.handle = threadHandle;
    context.isHijacked = false;

    // Get original thread context
    context.originalContext.ContextFlags = CONTEXT_FULL;
    if (NT_SUCCESS(NtGetContextThread(threadHandle, &context.originalContext)))
    {
        // Create new context
        CONTEXT newContext = context.originalContext;
#ifdef _WIN64
        newContext.Rip = reinterpret_cast<DWORD64>(newAddress);
#else
        newContext.Eip = reinterpret_cast<DWORD>(newAddress);
#endif

        // Set new context
        if (NT_SUCCESS(NtSetContextThread(threadHandle, &newContext)))
        {
            context.isHijacked = true;
            hijackedThreads.push_back(context);
            return true;
        }
    }

    CloseHandle(threadHandle);
    return false;
}

bool ProcessMemoryHandler::RestoreThread(DWORD threadId)
{
    auto *context = GetThreadContext(threadId);
    if (!context || !context->isHijacked)
    {
        return false;
    }

    // Restore original context
    bool success = NT_SUCCESS(NtSetContextThread(context->handle, &context->originalContext));
    if (success)
    {
        context->isHijacked = false;
    }

    return success;
}

void ProcessMemoryHandler::CleanupThreads()
{
    for (auto &context : hijackedThreads)
    {
        if (context.isHijacked)
        {
            NtSetContextThread(context.handle, &context.originalContext);
        }
        CloseHandle(context.handle);
    }
    hijackedThreads.clear();
}

bool ProcessMemoryHandler::ProtectMemory(LPVOID address, SIZE_T size, DWORD newProtection, PDWORD oldProtection)
{
    return VirtualProtectEx(processHandle, address, size, newProtection, oldProtection);
}

bool ProcessMemoryHandler::FindPatternAndAllocate(SIZE_T size, DWORD protection, LPVOID &outAddress)
{
    MEMORY_BASIC_INFORMATION mbi;
    LPVOID address = nullptr;

    // Find a suitable memory region
    if (!FindSuitableRegion(size, address))
    {
        return false;
    }

    // Allocate memory at the found address
    outAddress = VirtualAllocEx(
        processHandle,
        address,
        size,
        MEM_COMMIT | MEM_RESERVE,
        protection);

    return outAddress != nullptr;
}

bool ProcessMemoryHandler::IsAddressInRange(LPVOID address, SIZE_T size)
{
    for (const auto &region : allocatedRegions)
    {
        if (address >= region.first &&
            static_cast<BYTE *>(address) + size <= static_cast<BYTE *>(region.first) + region.second)
        {
            return true;
        }
    }
    return false;
}

bool ProcessMemoryHandler::FindSuitableRegion(SIZE_T size, LPVOID &outAddress)
{
    LPVOID address = nullptr;
    MEMORY_BASIC_INFORMATION mbi;

    // Pattern: Look for regions after commonly loaded DLLs
    while (VirtualQueryEx(processHandle, address, &mbi, sizeof(mbi)))
    {
        if (mbi.State == MEM_FREE && mbi.RegionSize >= size)
        {
            // Check if region is suitable (e.g., not too close to system DLLs)
            bool isSuitable = true;
            LPVOID testAddr = static_cast<BYTE *>(mbi.BaseAddress) - 0x1000;
            MEMORY_BASIC_INFORMATION testMbi;

            if (VirtualQueryEx(processHandle, testAddr, &testMbi, sizeof(testMbi)))
            {
                if (testMbi.Type == MEM_IMAGE)
                {
                    // Skip regions near loaded modules
                    isSuitable = false;
                }
            }

            if (isSuitable)
            {
                outAddress = mbi.BaseAddress;
                return true;
            }
        }
        address = static_cast<BYTE *>(mbi.BaseAddress) + mbi.RegionSize;
    }

    return false;
}

void ProcessMemoryHandler::XorBuffer(void *buffer, SIZE_T size)
{
    BYTE *bytes = static_cast<BYTE *>(buffer);
    for (SIZE_T i = 0; i < size; i++)
    {
        bytes[i] ^= XOR_KEY;
    }
}

bool ProcessMemoryHandler::ValidateMemoryRegion(LPVOID address, SIZE_T size)
{
    MEMORY_BASIC_INFORMATION mbi;
    if (!VirtualQueryEx(processHandle, address, &mbi, sizeof(mbi)))
    {
        return false;
    }

    return (mbi.State == MEM_COMMIT &&
            mbi.Protect != PAGE_NOACCESS &&
            mbi.Protect != PAGE_GUARD &&
            static_cast<BYTE *>(address) + size <=
                static_cast<BYTE *>(mbi.BaseAddress) + mbi.RegionSize);
}

ProcessMemoryHandler::ThreadContext *ProcessMemoryHandler::GetThreadContext(DWORD threadId)
{
    for (auto &context : hijackedThreads)
    {
        if (GetThreadId(context.handle) == threadId)
        {
            return &context;
        }
    }
    return nullptr;
}