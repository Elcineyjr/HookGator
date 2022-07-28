#include "pch.h"
#include "hde64.h"

#define TRAMPOLINE_MAX_SIZE (64 - sizeof(JMP_ABS))


#pragma once

#pragma pack(push, 1)
// 64-bit indirect absolute jump.
typedef struct _JMP_ABS
{
    UINT8  opcode0;     // FF25 00000000: JMP [+6]
    UINT8  opcode1;
    UINT32 dummy;
    UINT64 address;     // Absolute destination address
} JMP_ABS, * PJMP_ABS;

// 64-bit indirect absolute call.
typedef struct _CALL_ABS
{
    UINT8  opcode0;     // FF15 00000002: CALL [+6]
    UINT8  opcode1;
    UINT32 dummy0;
    UINT8  dummy1;      // EB 08:         JMP +10
    UINT8  dummy2;
    UINT64 address;     // Absolute destination address
} CALL_ABS;

typedef struct _JCC_ABS
{
    UINT8  opcode;      // 7* 0E:         J** +16
    UINT8  dummy0;
    UINT8  dummy1;      // FF25 00000000: JMP [+6]
    UINT8  dummy2;
    UINT32 dummy3;
    UINT64 address;     // Absolute destination address
} JCC_ABS;

typedef struct _JMP_REL
{
    UINT8  opcode;      // E9/E8 xxxxxxxx: JMP/CALL +5+xxxxxxxx
    UINT32 operand;     // Relative destination address
} JMP_REL, * PJMP_REL, CALL_REL;
#pragma pack(pop)

// Use _InterlockedCompareExchange64 insted of inline ASM (depends on compiler)
#define NO_INLINE_ASM
#undef UNICODE

// Size of each memory block. (= page size of VirtualAlloc)
#define MEMORY_BLOCK_SIZE 0x1000

// Max range for seeking a memory block. (= 1024MB)
#define MAX_MEMORY_RANGE 0x40000000

static LPVOID FindPrevFreeRegion(LPVOID pAddress, LPVOID pMinAddr, DWORD dwAllocationGranularity)
{
    ULONG_PTR tryAddr = (ULONG_PTR)pAddress;

    // Round down to the allocation granularity.
    tryAddr -= tryAddr % dwAllocationGranularity;

    // Start from the previous allocation granularity multiply.
    tryAddr -= dwAllocationGranularity;

    while (tryAddr >= (ULONG_PTR)pMinAddr)
    {
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery((LPVOID)tryAddr, &mbi, sizeof(mbi)) == 0)
            break;

        if (mbi.State == MEM_FREE)
            return (LPVOID)tryAddr;

        if ((ULONG_PTR)mbi.AllocationBase < dwAllocationGranularity)
            break;

        tryAddr = (ULONG_PTR)mbi.AllocationBase - dwAllocationGranularity;
    }

    return NULL;
}

// 32 bit relative jumps only cover a handful of addresses,
// so we set a pre-determined range of addresses
// to look for for writing the trampoline
LPVOID findFreeRegion(LPVOID lpMemRegion) {
    ULONG_PTR minAddr;
    ULONG_PTR maxAddr;
    LPVOID pOrigin = lpMemRegion;

    SYSTEM_INFO si;
    GetSystemInfo(&si);
    minAddr = (ULONG_PTR)si.lpMinimumApplicationAddress;
    maxAddr = (ULONG_PTR)si.lpMaximumApplicationAddress;

    // pOrigin ± 512MB
    if ((ULONG_PTR)pOrigin > MAX_MEMORY_RANGE && minAddr < (ULONG_PTR)pOrigin - MAX_MEMORY_RANGE)
        minAddr = (ULONG_PTR)pOrigin - MAX_MEMORY_RANGE;

    if (maxAddr > (ULONG_PTR)pOrigin + MAX_MEMORY_RANGE)
        maxAddr = (ULONG_PTR)pOrigin + MAX_MEMORY_RANGE;

    // Make room for MEMORY_BLOCK_SIZE bytes.
    maxAddr -= MEMORY_BLOCK_SIZE - 1;

    LPVOID pAlloc = pOrigin;
    while ((ULONG_PTR)pAlloc >= minAddr) {
        pAlloc = FindPrevFreeRegion(pAlloc, (LPVOID)minAddr, si.dwAllocationGranularity);
        if (pAlloc == NULL)
            break;

        pAlloc = VirtualAlloc(
            pAlloc, MEMORY_BLOCK_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (pAlloc != NULL)
            break;
    }

    return pAlloc;
}

LPBYTE relay;
BOOL createTrampoline(LPVOID target, LPVOID detour, LPVOID trampoline, LPVOID* original) {
    CALL_ABS call = {
        0xFF, 0x15, 0x00000002, // FF15 00000002: CALL [RIP+8]
        0xEB, 0x08,             // EB 08:         JMP +10
        0x0000000000000000ULL   // Absolute destination address
    };
    JMP_ABS jmp = {
        0xFF, 0x25, 0x00000000, // FF25 00000000: JMP [RIP+6]
        0x0000000000000000ULL   // Absolute destination address
    };
    JCC_ABS jcc = {
        0x70, 0x0E,             // 7* 0E:         J** +16
        0xFF, 0x25, 0x00000000, // FF25 00000000: JMP [RIP+6]
        0x0000000000000000ULL   // Absolute destination address
    };

    DWORD trampolineLength = 0, originalProtection;

    hde64s disasm;
    //printf("aa %d\n", sizeof(JMP_REL));
    while (trampolineLength < sizeof(JMP_REL)) {
        LPVOID instructionPointer = (LPVOID)((DWORD64)target + trampolineLength);
        trampolineLength += hde64_disasm(instructionPointer, &disasm);
    }
    __movsb((LPBYTE)trampoline, (LPBYTE)target, trampolineLength);

    jmp.address = (ULONG_PTR)target + trampolineLength;
    LPVOID    pCopySrc = &jmp;
    UINT      copySize = sizeof(jmp);

    __movsb((LPBYTE)trampoline + trampolineLength, (LPBYTE)pCopySrc, copySize);

    jmp.address = (ULONG_PTR)detour;
    UINT8 relayPos = trampolineLength + copySize;
    relay = (LPBYTE)trampoline + relayPos;

    memcpy(relay, &jmp, sizeof(jmp));

    DWORD oldProtect;
    if (!VirtualProtect(target, 0x1000, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        printf("bery vad\n");
        exit(1);
    }

    PJMP_REL pJmp = (PJMP_REL)target;
    pJmp->opcode = 0xE9;
    pJmp->operand = (UINT32)((LPBYTE)relay - ((LPBYTE)target + sizeof(JMP_REL)));

    VirtualProtect(target, 0x1000, oldProtect, &oldProtect);

    FlushInstructionCache(GetCurrentProcess(), target, sizeof(JMP_REL));

    *original = trampoline;

    return NULL;
}

 
//
//int main() {
//
//    LPVOID freeRegion = findFreeRegion(&MessageBoxA);
//    createTrampoline(MessageBoxA, DetouMessageBoxA, freeRegion, reinterpret_cast<LPVOID*>(&fpMessageBoxA));
//
//    printf("Found this free region: %llx\n", freeRegion);
//
//    MessageBoxA(NULL, "Testing", "Hook", MB_OK);
//    return 0;
//}