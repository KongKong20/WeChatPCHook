#pragma once

#define TARGET_NAME				"WeChat.exe"
#define DLL_NAME				"WechatClearContactDLL.dll"

BOOL LoadDll(DWORD pid);				//注入DLL
void UnloadDll(DWORD pid);				//卸载DLL
BOOL IsloadedDll(DWORD pid);			//检测DLL是否已经注入
DWORD FindNotLoadedDllPid();			//查找未加载DLL的进程
DWORD FindWeChatPid();					//查找未加载DLL的进程
