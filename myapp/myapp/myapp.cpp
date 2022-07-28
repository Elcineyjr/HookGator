// myapp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>

void PrintBytes(char* pLocation) {
    int i;
    for (i = 0; i < 10; i++) {
        unsigned char c = (pLocation)[i];
        printf("%02x ", c);
    }
    printf("\n");
}


int main()
{
    // Avoid race conditions
    Sleep(2000);
    MessageBoxA(NULL, "test", "test", MB_OK);

    if (((PBYTE)MessageBoxA)[0] == 0xE9) {
        printf("[+] MessageBoxA is being hooked!\n");
        PrintBytes((char*)MessageBoxA);

        MessageBoxExA(NULL, "test", "test", MB_OK, 0);
    }

    //getchar();
    //std::cout << "Hello World!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
