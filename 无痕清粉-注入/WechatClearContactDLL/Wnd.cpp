#include "stdafx.h"
#include "Wnd.h"
#include "Cmd.h"
#include "Contacts.h"
#include "VerifyUser.h"
#include "Message.h"
extern BOOL g_AutoChat;

int robotId = 0;
HWND hCurWnd;

void InitMsgWnd(HMODULE hModule)
{
	HookVerifyUserResult();
	GetContactsList();

	//注册窗口
	RegisterMsgWnd(hModule);
}

void RegisterMsgWnd(HMODULE hModule)
{
	//1  设计一个窗口类
	WNDCLASS wnd;
	wnd.style = CS_VREDRAW | CS_HREDRAW;//风格
	wnd.lpfnWndProc = WndProc;//窗口回调函数指针.
	wnd.cbClsExtra = NULL;
	wnd.cbWndExtra = NULL;
	wnd.hInstance = hModule;
	wnd.hIcon = NULL;
	wnd.hCursor = NULL;
	wnd.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wnd.lpszMenuName = NULL;
	wnd.lpszClassName = TEXT(SLAVE_NAME);

	//2  注册窗口类
	RegisterClass(&wnd);

	//3  创建窗口
	HWND hWnd = CreateWindow(
		TEXT(SLAVE_NAME),  //窗口类名
		TEXT(SLAVE_NAME),//窗口名
		WS_OVERLAPPEDWINDOW,//窗口风格
		10, 10, 500, 300, //窗口位置
		NULL,             //父窗口句柄
		NULL,             //菜单句柄
		hModule,        //实例句柄
		NULL              //传递WM_CREATE消息时的附加参数
	);

	//4  更新显示窗口
	ShowWindow(hWnd, SW_HIDE);
	UpdateWindow(hWnd);

	hCurWnd = hWnd;

	LPDWORD pId = NULL;
	GetWindowThreadProcessId(hWnd, pId);

	//5  消息循环（消息泵）
	MSG  msg = {};
	//   5.1获取消息
	while (GetMessage(&msg, 0, 0, 0))
	{
		//   5.2翻译消息
		TranslateMessage(&msg);
		//   5.3转发到消息回调函数
		DispatchMessage(&msg);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (Message == WM_COPYDATA)
	{
		COPYDATASTRUCT *pData = (COPYDATASTRUCT*)lParam;
		DealCmdFromMaster(pData->dwData, pData->cbData, pData->lpData);
	}

	return DefWindowProc(hWnd, Message, wParam, lParam);
}
