#include "stdafx.h"
#include "Inject.h"
#include <direct.h>
#include <stdlib.h>
#include <TlHelp32.h>
#include <stdio.h>

BOOL LoadDll(DWORD pId)
{
	if (0 == pId)
	{
		return FALSE;
	}

	if (IsloadedDll(pId))
	{
		return TRUE;
	}

	char* curPath = NULL;
	if ((curPath = _getcwd(NULL, 0)) == NULL)
	{
		//MessageBoxA(NULL, "获取当前工作目录失败", "错误", 0);
		return FALSE;
	}

	char dllFile[MAX_PATH] = { 0 };
	sprintf_s(dllFile, "%s\\%s", curPath, DLL_NAME);

	//打开进程
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pId);
	if (hProcess == NULL)
	{
		MessageBoxA(NULL, "进程打开失败", "错误", 0);
		return FALSE;
	}

	//在微信进程中申请内存
	LPVOID pAddress = VirtualAllocEx(hProcess, NULL, MAX_PATH, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (pAddress == NULL)
	{
		MessageBoxA(NULL, "内存分配失败", "错误", 0);
		return FALSE;
	}

	//写入dll路径到微信进程
	//DWORD dwWrite = 0;
	ULONG_PTR dwWrite = 0;
	if (WriteProcessMemory(hProcess, pAddress, dllFile, MAX_PATH, &dwWrite) == 0)
	{
		MessageBoxA(NULL, "路径写入失败", "错误", 0);
		return FALSE;
	}

	//获取LoadLibraryA函数地址
	FARPROC pLoadLibraryAddress = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
	if (pLoadLibraryAddress == NULL)
	{
		MessageBoxA(NULL, "获取LoadLibraryA函数地址失败", "错误", 0);
		return FALSE;
	}

	//远程线程注入dll
	HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryAddress, pAddress, 0, NULL);
	if (hRemoteThread == NULL)
	{
		MessageBoxA(NULL, "远程线程注入失败", "错误", 0);
		return FALSE;
	}

	CloseHandle(hRemoteThread);
	CloseHandle(hProcess);

	//MessageBoxA(NULL, "dll加载成功", "Tip", 0);
	return TRUE;
}


void UnloadDll(DWORD pId)
{
	if (pId == 0)
	{
		return;
	}

	//遍历模块
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pId);
	MODULEENTRY32 ME32 = { 0 };
	ME32.dwSize = sizeof(MODULEENTRY32);
	BOOL isNext = Module32First(hSnap, &ME32);
	BOOL flag = FALSE;
	while (isNext)
	{
		USES_CONVERSION;
		if (strcmp(W2A(ME32.szModule), DLL_NAME) == 0)
		{
			flag = TRUE;
			break;
		}
		isNext = Module32Next(hSnap, &ME32);
	}
	if (flag == FALSE)
	{
		MessageBoxA(NULL, "找不到目标模块", "错误", 0);
		return;
	}

	//打开目标进程
	HANDLE hPro = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pId);
	//获取FreeLibrary函数地址
	FARPROC pFun = GetProcAddress(GetModuleHandleA("kernel32.dll"), "FreeLibrary");

	//创建远程线程执行FreeLibrary
	HANDLE hThread = CreateRemoteThread(hPro, NULL, 0, (LPTHREAD_START_ROUTINE)pFun, ME32.modBaseAddr, 0, NULL);
	if (!hThread)
	{
		MessageBoxA(NULL, "创建远程线程失败", "错误", 0);
		return;
	}

	CloseHandle(hSnap);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	CloseHandle(hPro);
	MessageBoxA(NULL, "dll卸载成功", "Tip", 0);
}


BOOL IsloadedDll(DWORD dwProcessid)
{
	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
	//初始化模块信息结构体
	MODULEENTRY32 me32 = { sizeof(MODULEENTRY32) };
	//创建模块快照 1 快照类型 2 进程ID
	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessid);
	//如果句柄无效就返回false
	if (hModuleSnap == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(NULL, "创建模块快照失败", "错误", MB_OK);
		return FALSE;
	}

	//通过模块快照句柄获取第一个模块的信息
	if (!Module32First(hModuleSnap, &me32))
	{
		MessageBoxA(NULL, "获取第一个模块的信息失败", "错误", MB_OK);
		//获取失败则关闭句柄
		CloseHandle(hModuleSnap);
		return FALSE;
	}

	do
	{
		if (StrCmpW(me32.szModule, TEXT(DLL_NAME)) == 0)
		{
			return TRUE;
		}

	} while (Module32Next(hModuleSnap, &me32));

	return FALSE;
}

DWORD FindNotLoadedDllPid()
{
	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);

	HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (Process32First(hProcess, &pe32) == TRUE)
	{
		do
		{
			USES_CONVERSION;
			if (strcmp(TARGET_NAME, W2A(pe32.szExeFile)) == 0)
			{
				if (!IsloadedDll(pe32.th32ProcessID))
				{
					return pe32.th32ProcessID;
				}
			}
		} while (Process32Next(hProcess, &pe32));
	}

	return 0;
}

DWORD FindWeChatPid()
{
	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);

	HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (Process32First(hProcess, &pe32) == TRUE)
	{
		do
		{
			USES_CONVERSION;
			if (strcmp(TARGET_NAME, W2A(pe32.szExeFile)) == 0)
			{
				return pe32.th32ProcessID;
			}
		} while (Process32Next(hProcess, &pe32));
	}

	return 0;
}
