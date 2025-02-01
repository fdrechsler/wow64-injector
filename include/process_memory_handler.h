#pragma once
#include <windows.h>
#include <vector>
#include <memory>
#include <winternl.h>

class ProcessMemoryHandler
{
public:
    enum class MemoryAllocationType
    {
        MEM_COMMIT_PATTERN, // Uses pattern-based allocation
        MEM_COMMIT_STANDARD // Standard VirtualAllocEx
    };

    struct ThreadContext
    {
        HANDLE handle;
        CONTEXT originalContext;
        bool isHijacked;
    };

    ProcessMemoryHandler(HANDLE processHandle);
    ~ProcessMemoryHandler();

    // Memory allocation with pattern-based or standard method
    LPVOID AllocateMemory(
        SIZE_T size,
        DWORD protection,
        MemoryAllocationType allocType = MemoryAllocationType::MEM_COMMIT_STANDARD);

    // Write memory with XOR masking
    bool WriteMemory(
        LPVOID address,
        const void *buffer,
        SIZE_T size,
        SIZE_T *bytesWritten = nullptr);

    // Thread manipulation
    bool HijackThread(DWORD threadId, LPVOID newAddress);
    bool RestoreThread(DWORD threadId);
    void CleanupThreads();

    // Memory protection
    bool ProtectMemory(LPVOID address, SIZE_T size, DWORD newProtection, PDWORD oldProtection);

    // Pattern-based memory operations
    bool FindPatternAndAllocate(SIZE_T size, DWORD protection, LPVOID &outAddress);
    bool IsAddressInRange(LPVOID address, SIZE_T size);

private:
    HANDLE processHandle;
    std::vector<std::pair<LPVOID, SIZE_T>> allocatedRegions;
    std::vector<ThreadContext> hijackedThreads;
    const BYTE XOR_KEY = 0x55;

    // Internal helper methods
    bool FindSuitableRegion(SIZE_T size, LPVOID &outAddress);
    void XorBuffer(void *buffer, SIZE_T size);
    bool ValidateMemoryRegion(LPVOID address, SIZE_T size);
    ThreadContext *GetThreadContext(DWORD threadId);

    // NT API function pointers
    typedef NTSTATUS(NTAPI *NtGetContextThread_t)(HANDLE ThreadHandle, PCONTEXT Context);
    typedef NTSTATUS(NTAPI *NtSetContextThread_t)(HANDLE ThreadHandle, PCONTEXT Context);

    NtGetContextThread_t NtGetContextThread;
    NtSetContextThread_t NtSetContextThread;

    // Prevent copying
    ProcessMemoryHandler(const ProcessMemoryHandler &) = delete;
    ProcessMemoryHandler &operator=(const ProcessMemoryHandler &) = delete;
};