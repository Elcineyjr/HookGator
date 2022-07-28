// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

MESSAGEBOXA fpMessageBoxA = NULL;

// Detour function which overrides MessageBoxA
int WINAPI DetouMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
    return fpMessageBoxA(hWnd, "Nice try but im taking over now :)", "MessageBoxA is hooked!", uType);
}


DWORD WINAPI InitHooksThread(LPVOID param) {

    LPVOID freeRegion = findFreeRegion(&MessageBoxA);
    createTrampoline(MessageBoxA, DetouMessageBoxA, freeRegion, reinterpret_cast<LPVOID*>(&fpMessageBoxA));

    //printf("Found this free region: %llx\n", freeRegion);

    //MessageBoxA(NULL, "Testing", "Hook", MB_OK);
    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        //We are not interested in callbacks when a thread is created
        DisableThreadLibraryCalls(hModule);

        //We need to create a thread when initialising our hooks since
        //DllMain is prone to lockups if executing code inline.
        HANDLE hThread = CreateThread(nullptr, 0, InitHooksThread, nullptr, 0, nullptr);
        if (hThread != nullptr) {
            CloseHandle(hThread);
        }
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

