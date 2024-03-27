// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <windows.h>
#include <iostream>
#include "include/MinHook.h"
#include <vector>
#include <fstream>
#pragma comment(lib, "include/libMinHook.x64.lib")
char CtrlType[172*255];
typedef char*(__fastcall* getCtrlType)(__int64 unk, int ID);
getCtrlType pGetCtrlType = nullptr;
getCtrlType pGetCtrlTypeTarget;
TCHAR wDllFilePath[512] = { 0 };
char dllFilePath[512];
int getZeroes(char* a, int charArraySize) {
    int zeroCount = 0;
    for (int i = 0; i < charArraySize; i++) {
        if (a[i] == 0) {
			zeroCount++;
		}
	}
    return zeroCount;
}
std::string convertToString(char a[], int charArraySize)
{
    std::string s = "";
    int i = 0;
    while (i < charArraySize) {
        if (a[i] == 0) {
			break;
		}
        s.push_back(a[i]);
		i++;
	}
    return s;
}

// thank you jhrino
inline std::uint8_t* PatternScan(void* module, const char* signature)
{
    static auto pattern_to_byte = [](const char* pattern) {
        auto bytes = std::vector<int>{};
        auto start = const_cast<char*>(pattern);
        auto end = const_cast<char*>(pattern) + strlen(pattern);

        for (auto current = start; current < end; ++current) {
            if (*current == '?') {
                ++current;
                if (*current == '?')
                    ++current;
                bytes.push_back(-1);
            }
            else {
                bytes.push_back(strtoul(current, &current, 16));
            }
        }
        return bytes;
        };

    auto dosHeader = (PIMAGE_DOS_HEADER)module;
    auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dosHeader->e_lfanew);

    auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
    auto patternBytes = pattern_to_byte(signature);
    auto scanBytes = reinterpret_cast<std::uint8_t*>(module);

    auto s = patternBytes.size();
    auto d = patternBytes.data();

    for (auto i = 0ul; i < sizeOfImage - s; ++i) {
        bool found = true;
        for (auto j = 0ul; j < s; ++j) {
            if (scanBytes[i + j] != d[j] && d[j] != -1) {
                found = false;
                break;
            }
        }
        if (found) {
            return &scanBytes[i];
        }
    }
    return nullptr;
}

void get_contents()
{
    std::string path = convertToString(dllFilePath,512);
    size_t last_slash_idx = path.rfind('\\');
    std::string base_filename = path.substr(0, last_slash_idx);
    std::string filename = base_filename + "\\ctrltype.bin";
    // Open file
    std::ifstream infile(filename); // and since you want bytes rather than
    // characters, strongly consider opening the
    // File in binary mode with std::ios_base::binary
// Get length of file
    infile.seekg(0, std::ios::end);
    size_t length = infile.tellg();
    infile.seekg(0, std::ios::beg);

    // Don't overflow the buffer!
    if (length > sizeof(CtrlType)) {
        length = sizeof(CtrlType);
    }

    // Read file
    infile.read(CtrlType, length);
}

char* getCtrlTypeHook(__int64 unk,int ID)
{
	return &CtrlType[ID * 172];
}

bool initializeHooks()
{
    MH_STATUS hook = MH_Initialize();
    get_contents();
    if (hook != MH_OK) {
        std::cout << "Failed to initialize MinHook" << std::endl;
        return FALSE;
    }
    pGetCtrlTypeTarget = reinterpret_cast<getCtrlType>(PatternScan(GetModuleHandle(NULL), "48 63 C2 48 8D 0D ? ? ? 00 48 69 C0 AC 00 00 00"));
    MH_STATUS hooktest = MH_CreateHook(pGetCtrlTypeTarget, getCtrlTypeHook, NULL);

    if (hooktest != MH_OK) {
        std::cout << "Failed to create hook" << std::endl;
        return FALSE;
    }

    MH_STATUS test = MH_EnableHook(MH_ALL_HOOKS);

    if (test != MH_OK) {
        std::cout << "Failed to enable hook" << std::endl;
        return FALSE;
    }

    return TRUE;
}

DWORD WINAPI InitHook(LPVOID lpParameter) 
{
    std::string path = convertToString(dllFilePath, 512);
    get_contents();
    initializeHooks();
    while (true) {
        if (GetKeyState('P') & 0x8000) {
                get_contents();
		}
	}
    return TRUE;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        GetModuleFileName(hModule, wDllFilePath, 512);
        size_t* size = 0;
        wcstombs_s(size, dllFilePath, wDllFilePath, (size_t)512);
        CloseHandle(CreateThread(0, 0, InitHook, 0, 0, NULL));
        return TRUE;
    }
}

