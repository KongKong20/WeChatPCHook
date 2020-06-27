// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include <string>
using namespace std;

struct WxString
{
	// 存字符串
	wchar_t* buffer;

	// 存字符串长度
	int size;
	
	// 存字符串空间
	int capacity;

	// 8个空余的0
	char fill[8];
};

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
		wstring wsWxId = L"filehelper";
		wstring wsTextMsg = L"发送的消息";

		// 1、构造参数
		
		// 构造发送者结构
		WxString wxWxid = { 0 };
		wxWxid.buffer = (wchar_t*)wsWxId.c_str();
		wxWxid.size = wsWxId.size();
		wxWxid.capacity = wsWxId.capacity();

		// 构造消息结构
		WxString wxTextMsg = { 0 };
		wxTextMsg.buffer = (wchar_t*)wsTextMsg.c_str();
		wxTextMsg.size = wsTextMsg.size();
		wxTextMsg.capacity = wsTextMsg.capacity();

		// 构造空结构
		WxString wxNULL = { 0 };

		// 构造空buffer
		char buffer[0x738] = { 0 };

		// 2、获取DLL模块基址

		// 模块基址
		DWORD dllBaseAddress = (DWORD)GetModuleHandle(L"WeChatWin.dll");

		// 3、计算函数的内存地址

		// 函数偏移
		DWORD callOffset = 0x34CA50;
		// 函数内存地址
		DWORD callAddress = dllBaseAddress + callOffset;

		// 4、编写调用函数的代码
		/*
			参照源码
			5A17CA91    6A 01			push 0x1                            ; 参数5:1
			5A17CA93    8D43 34			lea eax,dword ptr ds:[ebx+0x34]
			5A17CA96    50              push eax                            ; 参数4:空结构
			5A17CA97    53              push ebx                            ; 参数3:发送的消息
			5A17CA98    8D55 9C         lea edx,dword ptr ss:[ebp-0x64]     ; 参数2:接收人
			5A17CA9B    8D8D C8F8FFFF   lea ecx,dword ptr ss:[ebp-0x738]	; 参数1:未知 可置空
			5A17CAA1    E8 AAFF2500     call WeChatWi.5A3DCA50				; 发送文本消息函数
			5A17CAA6    83C4 0C         add esp,0xC							; 平衡堆栈用
		*/
		__asm {
			// 参数5:1
			push 0x1;

			// 参数4:空结构
			lea eax, wxNULL;
			push eax;
			
			// 参数3:发送的消息
			lea ebx, wxTextMsg;
			push ebx;
			
			// 参数2:接收人
			lea edx, wxWxid;

			// 参数1:空buffer
			lea ecx, buffer;

			// 调用函数
			call callAddress;
			
			// 堆栈平衡
			add esp, 0xC;
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

