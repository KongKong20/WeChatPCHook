// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

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
		
		// 2、计算函数的内存地址
		
		// 第一个函数偏移
		DWORD firstCallOffset = 0x22C010;
		// 第一个函数内存地址
		DWORD firstCallAddress = dllBaseAddress + firstCallOffset;
		// 第二个函数偏移
		DWORD secondCallOffset = 0x344170;
		// 第二个函数内存地址
		DWORD secondCallAddress = dllBaseAddress + secondCallOffset;

		// 3、编写调用函数的代码
		/*
			参照源码
			5C00FA88    E8 83C5FFFF     call WeChatWi.5C00C010
			5C00FA8D    8BC8            mov ecx,eax
			5C00FA8F    E8 DC461100     call WeChatWi.5C124170
		*/
		__asm {
			call firstCallAddress;
			mov	 ecx, eax;
			call secondCallAddress;
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

