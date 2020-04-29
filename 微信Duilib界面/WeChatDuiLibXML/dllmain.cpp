// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include <string>
#include <SHLWAPI.H>
using namespace std;
#pragma comment(lib, "Shlwapi.lib")

//#define OFFSET_DUILIB_HOOK_ADDRESS	0x6E5540
#define OFFSET_DUILIB_HOOK_ADDRESS	0x81DC70

DWORD jmpBackAddress = 0;

VOID SaveDuiLibXML(BYTE* xmlData, DWORD xmlSize, WCHAR* xmlPathFileName)
{
	wstring filepath = L"D:\\WeChatDuiLibXMLFile";
	if (!PathIsDirectory(filepath.c_str()))
	{
		CreateDirectory(filepath.c_str(), NULL);
	}

	wstring wsXmlPathFileName = xmlPathFileName;
	int index = wsXmlPathFileName.find(L"/");
	if (index < 0) index = wsXmlPathFileName.find(L"\\");

	if (index > 0)
	{
		wstring wsXmlPath = wsXmlPathFileName.substr(0, index);
		filepath += L"\\" + wsXmlPath;
		if (!PathIsDirectory(filepath.c_str()))
		{
			CreateDirectory(filepath.c_str(), NULL);
		}
	}

	wstring wsXmlFileName = wsXmlPathFileName.substr(index+1);
	filepath += L"\\" + wsXmlFileName;

	OutputDebugString(filepath.c_str());

	HANDLE hFile = CreateFile(filepath.c_str(),
		GENERIC_WRITE, 0, NULL,
		CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);

	DWORD dwBytesWriten = 0;
	WriteFile(hFile, xmlData, xmlSize*2, &dwBytesWriten, NULL);

	CloseHandle(hFile);
}

__declspec(naked) VOID ListenDuiLibXML()
{
	_asm
	{
		mov ebx, dword ptr ss : [ebp - 0x4];
		xor ecx, ecx;

		pushfd;
		pushad;

		mov eax, [ebp];
		push [eax+0x8];
		push edx;
		push [ebx];
		call SaveDuiLibXML;
		add esp, 0xC;

		popad
		popfd

		jmp jmpBackAddress
	}
}

VOID HookDuiLibXML(HMODULE hModule)
{
	DWORD wxBaseAddress = 0;
	while (wxBaseAddress == 0)
	{
		Sleep(100);
		wxBaseAddress = (DWORD)GetModuleHandle(TEXT("WeChatWin.dll"));
	}

	DWORD hookAddress = wxBaseAddress + 0x81DC70;
	jmpBackAddress = hookAddress + 5;

	BYTE jmpCode[5] = { 0xE9 };
	*((int*)& jmpCode[1]) = (DWORD)ListenDuiLibXML - hookAddress - 5;
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)hookAddress, jmpCode, 5, 0);
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
		HANDLE hANDLE = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HookDuiLibXML, hModule, NULL, 0);
		if (hANDLE != 0)
		{
			CloseHandle(hANDLE);
		}
	}
	break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

