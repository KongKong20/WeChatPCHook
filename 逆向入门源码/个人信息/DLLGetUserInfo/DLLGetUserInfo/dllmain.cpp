// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

#include <string>
using namespace std;

DWORD GetIntByAddress(DWORD address)
{
	DWORD intValue = 0;

	intValue = *(DWORD*)address;

	return intValue;
}

string GetStringByAddress(DWORD address)
{
	string sValue = "";

	char cValue[500] = { 0 };
	memcpy(cValue, (const void*)address, 500);
	sValue = string(cValue);

	return sValue;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
	// dll被加载进内存后，会执行到这里
    case DLL_PROCESS_ATTACH:
	{
		// 1、获取DLL模块基址

		// 模块基址
		DWORD dllBaseAddress = (DWORD)GetModuleHandle(L"WeChatWin.dll");

		// 2、获取用户名
		
		// 用户名偏移
		DWORD wxNameOffset = 0x16B498C;
		// 用户名内存地址
		DWORD wxNameAddress = dllBaseAddress + wxNameOffset;
		// 用户名
		string sWxName = GetStringByAddress(wxNameAddress);

		// 3、获取WXID

		// WXID指针偏移
		DWORD wxIDPointerOffset = 0x16B4D90;
		// WXID指针地址
		DWORD wxIDPointerAddress = dllBaseAddress + wxIDPointerOffset;
		// WXID内存地址
		DWORD wxIDAddress = GetIntByAddress(wxIDPointerAddress);
		// WXID
		string sWxId = GetStringByAddress(wxIDAddress);

		// 4、显示结果

		// 用户信息
		string csUserInfo = string("用户信息： 用户名：") + sWxName + " 用户ID: " + sWxId;
		MessageBoxA(NULL, csUserInfo.c_str(), "用户信息", 0);
	}
	break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

