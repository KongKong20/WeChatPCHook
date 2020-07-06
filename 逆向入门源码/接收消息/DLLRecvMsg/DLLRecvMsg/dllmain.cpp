// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include <string>
using namespace std;

// 模块基址
DWORD dllBaseAddress = (DWORD)GetModuleHandle(L"WeChatWin.dll");

// 接收消息hook位置偏移
DWORD recievedHookOffset = 0x329743;

// 接收消息hook位置的内存地址
DWORD recievedHookAddress = dllBaseAddress + recievedHookOffset;

// 被覆盖的数据的偏移
DWORD coeverDataOffset = 0x14003C0;

// 被覆盖的数据的内存地址
DWORD coeverDataAddress = dllBaseAddress + coeverDataOffset;

// Hook下一个指令的地址
DWORD recievedHookRetAddress = recievedHookAddress + 5;
CHAR originalRecieveCode[5] = { 0 };

void DealRecievedMessage(WCHAR* sender, WCHAR* msg)
{
	wstring wsSender = sender;
	wstring wsMsg = msg;
	wstring wsShowMsg = L"发送者：" + wsSender + L" 消息： " + wsMsg;
	MessageBox(NULL, wsShowMsg.c_str(), L"收到消息", 0);
}

__declspec(naked) void ListenRecievedMessage()
{
	__asm
	{
		//保存现场
		pushad;
		pushfd;

		// 发送的消息
		push [ebp - 0x1F8];
		// 发送的人
		push [ebp - 0x220];
		
		// 调用处理消息的函数
		call DealRecievedMessage;

		// 平衡堆栈 一个参数占0x4，两个参数就是0x8
		add esp, 0x8;

		//恢复现场
		popfd;
		popad;
	
		// 重新执行被覆盖的 
		// push WeChatWi.5B4903C0
		push coeverDataAddress;

		// 返回hook的下一条指令
		jmp recievedHookRetAddress
	}
}

void HookRecievedMessage()
{
	// 跳转需要五个字节
	BYTE jmpCode[5] = { 0 };

	// 第一个字节填 E9，对应汇编的jmp
	jmpCode[0] = 0xE9;

	// 后面四个字节，填要跳转的位置，之所以减5，是因为当前的指令占五个字节
	*(DWORD*)&jmpCode[1] = (DWORD)ListenRecievedMessage - recievedHookAddress - 5;

	// 把老的指令读出来存好，方便恢复
	ReadProcessMemory(GetCurrentProcess(), (LPVOID)recievedHookAddress, originalRecieveCode, 5, 0);

	// 把新的执行写到hook的位置
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)recievedHookAddress, jmpCode, 5, 0);
}

void UnHookRecievedMessage()
{
	// 回复被覆盖的指令
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)recievedHookAddress, originalRecieveCode, 5, 0);
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
		HookRecievedMessage();
	}
	break;
    case DLL_PROCESS_DETACH:
	{
		UnHookRecievedMessage();
	}
    break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
	break;
    }
    return TRUE;
}

