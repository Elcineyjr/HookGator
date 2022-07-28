#pragma once

#include "pch.h"

typedef int (WINAPI* MESSAGEBOXA)(HWND, LPCSTR, LPCSTR, UINT);
extern MESSAGEBOXA fpMessageBoxA;

LPVOID findFreeRegion(LPVOID lpMemRegion);
BOOL createTrampoline(LPVOID target, LPVOID detour, LPVOID trampoline, LPVOID* original);
